# include "core.hpp"
# include "checkFile.hpp"
# include "ConfigLoader.hpp"
# include "ConfigChecker.hpp"
#include "httpError.hpp"
#include <string>
# include <sys/stat.h>

void			core(PollSet& pollset, ServerSocket *serv, ConnSocket *connected)
{
	string			reqTarget = connected->ReqH.getRequsetTarget();
	string			ext	= getExt(reqTarget);
	status_code_t	status = 42;

	status = writeResponseBody(connected, reqTarget);
	connected->ResH.setStatusCode(status);
	if (CONF->MIME.find(ext) != CONF->MIME.end())
		connected->ResH["Content-Type"]	= CONF->MIME[ext];
	else
		connected->ResH["Content-Type"] = connected->conf->default_type;
	if (!connected->ResB.getContent().empty())
		connected->ResH["Content-Length"]	= toString(connected->ResB.getContent().length());
	if (status == 200 && getExt(reqTarget) == "py")
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


status_code_t	writeResponseBody(ConnSocket* connected, const string& reqTarget)
{
	struct stat 	s;
	status_code_t	status;
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

		try							{ indexfile = findIndexFile(connected->conf, prefix, uri); }
		catch (httpError& e)		{ throw; }

		if (indexfile.back() == '/')
		{
			if (connected->conf->auto_index)
			{
				connected->ResB.setContent(directoryListing(prefix, indexfile));
				return 200;
			}
			else
				throw forbidden();
		}
		filename = prefix + indexfile;
	}
	status = connected->ResB.readFile(filename);

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
