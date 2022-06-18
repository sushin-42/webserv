# include "core.hpp"

extern map<string, string> MIME;

void			core(PollSet& pollset, ServerSocket *serv, ConnSocket *connected)
{
	string			filepath = connected->ReqH.getRequsetTarget();
	string			ext	= getExt(filepath);
	status_code_t	status = 42;

	status = writeResponseBody(connected, filepath);
	connected->ResH.setStatusCode(status);
	if (MIME.find(ext) != MIME.end())
		connected->ResH["Content-Type"]	= MIME[ext];	// No matching MIME
	//IMPL: else, load default_type
	if (!connected->ResB.getContent().empty())
		connected->ResH["Content-Length"]	= toString(connected->ResB.getContent().length());
	if (status == 200 && getExt(filepath) == "py")
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


status_code_t	writeResponseBody(ConnSocket* connected, const string& filepath)
{
	status_code_t	status = connected->ResB.readFile(connected->conf->root + filepath);
	if (status == 404)
		connected->ResB.readFile(connected->conf->root + "/404/404.html");

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
