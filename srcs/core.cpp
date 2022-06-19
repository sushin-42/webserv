# include "core.hpp"
# include "checkFile.hpp"
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
	status_code_t	status;
	struct stat 	s;
	string			filepath = connected->conf->root + uri;

	stat(filepath.c_str(), &s);		//NOTE: stat 이 앞에서 체크한거랑 달라졌을 수 있음!
	if (S_ISDIR(s.st_mode) && connected->conf->auto_index)
	{
		connected->ResB.setContent(directoryListing(connected->conf->root, uri));
		status = 200;
	}
	else
		status = connected->ResB.readFile(filepath);

	if (status == 404)
		connected->ResB.readFile(connected->conf->root + "/404/404.html");		//FIXIT

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
