#include "CGI.hpp"
#include "Config.hpp"
#include "ConnSocket.hpp"
# include "ResBody.hpp"
# include "ResHeader.hpp"
#include "utils.hpp"

status_code_t	writeResponseBody(ConnSocket* connected, const string& filepath);
void			writeResponseHeader(ConnSocket* connected, status_code_t status);

void	core(ServerSocket *serv, ConnSocket *connected)
{
	status_code_t	status = 42;
	string	filepath = connected->ReqH.getRequsetTarget();
	string	ext	= getExt(filepath);


	status = writeResponseBody(connected, filepath);
	if (MIME.find(ext) != MIME.end())
		connected->ResH["Content-Type"]	= MIME[ext];	// No matching MIME
	if (getExt(filepath) == "py")
		CGIRoutines(serv, connected);
	if (!connected->ResB.getContent().empty() &&
		!connected->ResH.exist("content-length"))
		connected->ResH["Content-Length"]	= toString(connected->ResB.getContent().length());
	//1.client-redir	: if no Status -> set 302, Found	//@
	//					  if Content-length not in script, set in clientRedir() //@
	//            		  if CL in script and not matched with body, timeout occur (see LIGHTTPD / what if APACHE?) //!

	//2.document		: Content-length set by script, if exists. //@
	//		   			  else, APACHE: chunk-encoding
	// 							LIGHTTPD: get length of CGI body //@
	// 					  if CL in script not matched with body, timeout occur (see LIGHTTPD what if APACHE?) //!

	//3.local-redir		: if (Status := 200 | "" ) Content-Length, Type are newly set by local-redir-page. Location removed.
	//					  else:	 no redirection. if no redir, No header modification.

	writeResponseHeader(connected, status);

}



void	core_wrapper(ServerSocket *serv, ConnSocket *connected)
{

	core(serv, connected);
	connected->ResH.makeStatusLine();
	connected->ResH.integrate();
}


status_code_t	writeResponseBody(ConnSocket* connected, const string& filepath)
{
	status_code_t	status = connected->ResB.readFile(root + filepath);
	if (status == 404)
		connected->ResB.readFile(root + "/404/404.html");

	return status;
}

void	writeResponseHeader(ConnSocket* connected, status_code_t status)
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
		connected->ResH.setStatusCode(status);
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
