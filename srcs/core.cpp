# include "core.hpp"
# include "checkFile.hpp"
#include "httpError.hpp"
# include <sys/stat.h>

extern map<string, string> MIME;

void			core(PollSet& pollset, ServerSocket *serv, ConnSocket *connected)
{
	string			uri = connected->ReqH.getRequsetTarget();
	string			ext	= getExt(uri);
	status_code_t	status = 42;

	status = writeResponseBody(connected, uri);
	connected->ResH.setStatusCode(status);
	if (MIME.find(ext) != MIME.end())
		connected->ResH["Content-Type"]	= MIME[ext];	// No matching MIME
	else
		// CONF->
	//IMPL: else, load default_type
	if (!connected->ResB.getContent().empty())
		connected->ResH["Content-Length"]	= toString(connected->ResB.getContent().length());
	if (status == 200 && getExt(uri) == "py")
	{
		connected->ResB.clear();
		connected->ResH.removeKey("content-length");
		CGIRoutines(pollset, serv, connected, NULL);
		return ;
	}
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


status_code_t	writeResponseBody(ConnSocket* connected, const string& uri)
{
	struct stat 	s;
	status_code_t	status;
	string			indexfile;
	string			filepath = connected->conf->root + uri;

	try						{ s = _checkFile(filepath); }
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
		try							{ indexfile = findIndexFile(connected->conf, uri); }
		catch (httpError& e)		{ throw; }

		if (indexfile.back() == '/')
		{
			if (connected->conf->auto_index)
			{
				connected->ResB.setContent(directoryListing(connected->conf->root, indexfile));
				return 200;
			}
			else
				throw forbidden();
		}
		filepath = connected->conf->root + indexfile;
	}
	status = connected->ResB.readFile(filepath);

	return status;
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
