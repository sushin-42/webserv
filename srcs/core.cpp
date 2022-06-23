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

	stream->core();
	if (connected->pending)
		return;

	/*
	   cannot enter if readMore(),
	   if reach here after CGI local-redir(),   [ core_wrapper(CGIpipe) => CGIpipe->core() => processOutputHeader() => localRedir() => connected->core() ]
	   maybe it has Response Header Fields.
	*/
	if (!connected->ResH.getHeaderField().empty())
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
