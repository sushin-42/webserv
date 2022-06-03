#include "CGI.hpp"
#include "Config.hpp"
#include "ConnSocket.hpp"
#include "Poll.hpp"
# include "ResBody.hpp"
# include "ResHeader.hpp"
#include "utils.hpp"

status_code_t	writeResponseBody(ConnSocket* connected, const string& filepath);
void			writeResponseHeader(ConnSocket* connected);

void	core(PollSet& pollset, ServerSocket *serv, ConnSocket *connected)
{
	string			filepath = connected->ReqH.getRequsetTarget();
	string			ext	= getExt(filepath);
	status_code_t	status = 42;

	status = writeResponseBody(connected, filepath);
	connected->ResH.setStatusCode(status);
	if (MIME.find(ext) != MIME.end())
		connected->ResH["Content-Type"]	= MIME[ext];	// No matching MIME
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
	status_code_t	status = connected->ResB.readFile(root + filepath);
	if (status == 404)
		connected->ResB.readFile(root + "/404/404.html");

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
	connected->ResH["Connection"]			= "close";
	connected->ResH["Server"]				= "Webserv 0.1";
	// ResH["Keep-Alive"]			=
	// ResH["Last-Modified"]		=
	// ResH["E-Tag"]				=
	// ResH["Transfer-Encoding"]	=
}
