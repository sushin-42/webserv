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


//! check if OK for CGI local redir...
void			core(Stream* stream)
{
	stream->core();
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
