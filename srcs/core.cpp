# include "core.hpp"
#include "Exceptions.hpp"
#include "FileStream.hpp"
#include "Pipe.hpp"
# include "checkFile.hpp"
# include "ConfigLoader.hpp"
# include "ConfigChecker.hpp"
#include "httpError.hpp"
#include <string>
# include <sys/stat.h>

bool	createPUToutputFile(ConnSocket* connected, const string filename);
void	writePUToutputFile(ConnSocket* c, const string& fn);


string			createInputFileStream(ConnSocket* connected, const string& filename)
{
	FileStream* f = new FileStream(filename);
	f->open(O_RDONLY);

	connected->linkInputFile = f;
	f->linkConn = connected;
	POLLSET->enroll(f, POLLIN);

	throw readMore();
}

void			readInputFileStream(FileStream* inputFileStream)
{
	ssize_t byte;
	switch (byte = inputFileStream->read())
	{
	case -1:
		TAG(core#, core); cout << RED("Unexcpected error from file: ") << inputFileStream->getFilename() << endl;
		inputFileStream->close();
		throw internalServerError();

	case 0:		/* close file, process output */
		TAG(core#, core); cout << GRAY("file closed: ") << inputFileStream->getFilename() << endl;
		inputFileStream->close();
		break;

	default:	/* content appended */
		throw readMore();
	}
}


//! check if OK for CGI local redir...
void			core(/* ServerSocket *serv,  */Stream* stream)
{
	ConnSocket*	connected = CONVERT(stream, ConnSocket);
	Pipe*		inputPipe = CONVERT(stream, Pipe);
	FileStream*	inputFileStream = CONVERT(stream, FileStream);

	struct stat s;


//*---------------------------------ConnSocket-------------------------------*//
	if (connected)
	{
		ServerSocket*	serv = connected->linkServerSock;
		string			reqTarget = connected->ReqH.getRequsetTarget();
		string			filename;
		string			ext;

		bool			alreadyExist = false;

		try 						{ filename = getFileName(connected, reqTarget); }
		catch (httpError& e)		{ throw; }


		if (connected->ReqH.getMethod() == "PUT")
		{
			try						{ alreadyExist = createPUToutputFile(connected, filename); (void)alreadyExist; }
			catch (Conflict& e)		{ throw; }

			connected->ResH.setStatusCode(alreadyExist ? 204 : 201);
			connected->ResH.setReasonPhrase(alreadyExist ? "No Content" : "Created");
			connected->ResH.setDefaultHeaders();

			return;
		}

		try							{ s =_checkFile(filename);
									  if (S_ISDIR(s.st_mode) && filename.back() != '/')
									  		throw movedPermanently("http://" + connected->ReqH["Host"] + reqTarget + '/');
									}
		catch (httpError& e)		{
									  throw;
									}

		try							{ filename = checkIndex(connected->conf, filename); }
		catch (httpError& e)		{ throw; }
		catch (autoIndex& a)		{ connected->ResH.setStatusCode(200);
									  connected->ResH.setDefaultHeaders();					//FIXIT: prefix
									  connected->ResB.setContent(directoryListing(a.path, "/"));	/* alias case: need to append Loc URI || or req Target ? */
									  connected->ResH["Content-Length"]	= toString(connected->ResB.getContent().length());
									  connected->ResH["Content-Type"]		= "text/html";
									  return ;
									}

		if (getExt(filename) == "py")	//! check case (.py == DIRECTORY)
		{
			connected->ResB.clear();
			connected->ResH.removeKey("content-length");
			if (connected->linkInputPipe == NULL)
				return createCGI(serv, connected);
		}
		createInputFileStream(connected, filename);	//readMore
	}
//%-----------------------------Input FileStream-----------------------------%//
	else if (inputFileStream)
	{
		connected = inputFileStream->linkConn;
		string ext		= getExt(connected->ReqH.getRequsetTarget());

		try					{ readInputFileStream(inputFileStream); }
		catch (exception& e){ throw; }	// ( readMore | 500 | processing Response)

		connected->ResB.setContent(inputFileStream->content);
		connected->ResH.setStatusCode(200);
		connected->ResH["Content-Type"]	= CONF->MIME.find(ext) != CONF->MIME.end() ?
										CONF->MIME[ext] : connected->conf->default_type;

		if (!connected->ResB.getContent().empty())
			connected->ResH["Content-Length"]	= toString(connected->ResB.getContent().length());
	}

//,------------------------------Input CGI Pipe------------------------------,//
	else if (inputPipe)
	{
		readFromCGI(inputPipe);
	}
}



void	core_wrapper(Stream* stream)
{
	FileStream*	inputFileStream = CONVERT(stream, FileStream);
	Pipe*		inputPipe = CONVERT(stream, Pipe);
	ConnSocket*	connected = CONVERT(stream, ConnSocket);

	if (!connected)
	{
		if		(inputPipe)			connected = inputPipe->linkConn;
		else if (inputFileStream)	connected = inputFileStream->linkConn;
	}

	core(stream);
	if (connected->pending)
		return;

	if (!connected->ResH.getHeaderField().empty())	/* cannot enter if readMore() */
	{
		writeResponseHeader(connected);
		connected->ResH.makeStatusLine();
		connected->ResH.integrate();
	}
}

string	getFileName(ConnSocket* connected, const string& reqTarget)
{
	string			indexfile;
	string			prefix;
	string			uri;

	pair <string, string> p = CHECK->routeRequestTarget(connected->conf, reqTarget);
	prefix = p.first;
	uri = p.second;

	string			filename = prefix + uri;

	return filename;
}


void	writeResponseHeader(ConnSocket* connected)
{
	connected->ResH.setHTTPversion("HTTP/1.1");
	if (connected->ResH.exist("status") == true)
	{
		pair<status_code_t, string>	sr = checkStatusField(connected->ResH["Status"]);
		connected->ResH.setStatusCode(sr.first);
		connected->ResH.setReasonPhrase(sr.second);
		connected->ResH.removeKey("Status");
	}
	else
	{
		switch (connected->ResH.getStatusCode())
		{
		case 200:	connected->ResH.setReasonPhrase("OK");			break;
		case 404:	connected->ResH.setReasonPhrase("Not Found");	break;
		/* and so on ... */
		}
	}
	connected->ResH.setDefaultHeaders();
}



status_code_t	deleteFile(pair<string, string> p)
{
	int		ret;
	struct 	stat s;
	string	filename = p.first + p.second;

	try						{ s = _checkFile(filename); }
	catch (httpError& e)	{ throw; }	// for 403

	if (S_ISDIR(s.st_mode) && filename.back() != '/')
		return 409; // throw?

	ret = remove(filename.c_str());
	if (ret == -1)
		return 404; // throw?
	return 204; // throw?
}
bool	createPUToutputFile(ConnSocket* connected, const string filename)
{

	FileStream* f = new FileStream(filename);

	bool alreadyExist = (access(filename.c_str(),F_OK) == 0);

	f->open(O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK);
	if (f->getFD() == -1)
		throw Conflict();

	connected->linkOutputFile = f;
	f->linkConn = connected;

	POLLSET->enroll(f, POLLOUT);

	return alreadyExist;

}

