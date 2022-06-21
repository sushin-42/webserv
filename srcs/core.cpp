# include "core.hpp"
#include "Exceptions.hpp"
#include "FileStream.hpp"
# include "checkFile.hpp"
# include "ConfigLoader.hpp"
# include "ConfigChecker.hpp"
#include "httpError.hpp"
#include <string>
# include <sys/stat.h>

string			createInputFileStream(ConnSocket* connected, const string& reqTarget, PollSet& pollset)
{
	// string	autoindexBody;
	string filename;
	filename = getFileName(connected, reqTarget);	//FIXME: too many task here: check file, check index, auto index, ... throw httpError
	FileStream* f = new FileStream(filename);
	f->open(O_RDONLY);

	if (filename.back() == '/')
	{
		if (connected->conf->auto_index)		// DO NOT READ. just do directory listing INSTANTLY.
		{
			f->close();
			delete f;

			throw autoIndex();
		}
		else
			throw forbidden();
	}
	connected->linkInputFile = f;
	f->linkConn = connected;
	pollset.enroll(f);

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
				/* FIXIT: now linkInputFile/linkInputPipe need to be drop by dropLink() */

		TAG(core#, core); cout << GRAY("file closed: ") << inputFileStream->getFilename() << endl;
		inputFileStream->close();
		break;

	default:	/* content appended */
		throw readMore();
	}
}

void			core(PollSet& pollset, ServerSocket *serv, ConnSocket *connected)
{
	string			reqTarget = connected->ReqH.getRequsetTarget();
	string			filename;
	string			ext;

	FileStream*		inputFileStream = connected->linkInputFile;

	if (!inputFileStream)
	{
		try					{ createInputFileStream(connected, reqTarget, pollset); }
		catch (autoIndex& a){ goto _end; }
	}
	else if (inputFileStream)
	{
		try					{readInputFileStream(inputFileStream);}
		catch (exception& e){ throw; }	// ( readMore | 500 | processing Response)
	}

	connected->ResB.setContent(inputFileStream->content);
	connected->ResH.setStatusCode(200);

	ext = getExt(inputFileStream->getFilename());
	connected->ResH["Content-Type"]	= CONF->MIME.find(ext) != CONF->MIME.end() ?
									  CONF->MIME[ext] : connected->conf->default_type;

	if (!connected->ResB.getContent().empty())
		connected->ResH["Content-Length"]	= toString(connected->ResB.getContent().length());

	if (getExt(inputFileStream->getFilename()) == "py")
	{
		connected->ResB.clear();
		connected->ResH.removeKey("content-length");
		CGIRoutines(pollset, serv, connected, NULL);
		return ;
	}
_end:;
}



void	core_wrapper(PollSet& pollset, ServerSocket *serv, ConnSocket *connected, Pipe* CGIpipe)
{
	if (!CGIpipe)
		core(pollset, serv, connected);
	else
	{
		CGIRoutines(pollset, serv, connected, CGIpipe);
		if (connected->pending)
			return;
	}
	if (!connected->ResH.getHeaderField().empty())
	{
		writeResponseHeader(connected);
		connected->ResH.makeStatusLine();
		connected->ResH.integrate();
	}
}


string	getFileName(ConnSocket* connected, const string& reqTarget)
{
	struct stat 	s;
	string			indexfile;
	string			prefix;
	string			uri;

	pair <string, string> p = CHECK->routeRequestTarget(connected->conf, reqTarget);
	prefix = p.first;
	uri = p.second;

	string			filename = prefix + uri;

	try						{ s = _checkFile(filename); }
	catch (httpError& e)	{ throw; }	// for 404

	if (S_ISDIR(s.st_mode))
	{
	/**========================================================================
	 * @  if FOUND final (deepest) index file
	 * @  	if final index file was directory
	 * @		auto index on ? directory listing() : forbidden();
	 * @	else
	 * @		if final index file forbidden? forbidden();
	 * @		else print index file;
	 * '  else NOT FOUND
	 * '	auto index on ? directory listing() : forbidden();
	 *========================================================================**/
		if (reqTarget.back() != '/')
			throw movedPermanently("http://" + connected->ReqH["Host"] + reqTarget + '/');

		try							{ indexfile = findIndexFile(connected->conf, filename); }
		catch (httpError& e)		{ throw; }

		if (indexfile.back() == '/')
		{
			if (connected->conf->auto_index)
			{
				connected->ResH.setStatusCode(200);
				connected->ResH.setDefaultHeaders();
				connected->ResB.setContent(directoryListing(indexfile, prefix));	/* alias case: need to append Loc URI || or req Target ? */
				connected->ResH["Content-Length"]	= toString(connected->ResB.getContent().length());
				connected->ResH["Content-Type"]		= "text/html";
				return indexfile;
			}
			else
				throw forbidden();
		}
		filename = indexfile;
	}
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
