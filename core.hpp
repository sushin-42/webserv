#include "CGI.hpp"
#include "Config.hpp"
#include "ConnSocket.hpp"
#include "Poll.hpp"
# include "ResBody.hpp"
# include "ResHeader.hpp"
#include "utils.hpp"

status_code_t	writeResponseBody(ResBody& ResB, const string& filepath);
void			writeResponseHeader(ResHeader& ResH, status_code_t status);

void	core(PollSet& pollset,
			 ServerSocket *serv, ConnSocket *connected,
			 ReqHeader &ReqH, const ReqBody &ReqB,
			 ResHeader &ResH, ResBody &ResB)
{
	status_code_t	status = 42;
	string	filepath = ReqH.getRequsetTarget();
	string	ext	= getExt(filepath);


	status = writeResponseBody(ResB, filepath);
	if (MIME.find(ext) != MIME.end())
		ResH["Content-Type"]	= MIME[ext];	// No matching MIME
	if (getExt(filepath) == "py")
		CGIRoutines(pollset, serv, connected, ReqH, ReqB, ResH, ResB);
	if (!ResB.getContent().empty() && !ResH.exist("content-length"))
		ResH["Content-Length"]	= toString(ResB.getContent().length());
	//1.client-redir	: if no Status -> set 302, Found	//@
	//					  if Content-length not in script, set in clientRedir() //@
	//            		  if CL in script and not matched with body, timeout occur (see LIGHTTPD / what if APACHE?) //!

	//2.document		: Content-length set by script, if exists. //@
	//		   			  else, APACHE: chunk-encoding
	// 							LIGHTTPD: get length of CGI body //@
	// 					  if CL in script not matched with body, timeout occur (see LIGHTTPD what if APACHE?) //!

	//3.local-redir		: if (Status := 200 | "" ) Content-Length, Type are newly set by local-redir-page. Location removed.
	//					  else:	 no redirection. if no redir, No header modification.

	writeResponseHeader(ResH, status);

}



void	core_wrapper(PollSet& pollset,
					ServerSocket *serv, ConnSocket *connected,
			 		ReqHeader &ReqH, const ReqBody &ReqB,
			 		ResHeader &ResH, ResBody &ResB)
{

	core(pollset, serv, connected, ReqH, ReqB, ResH, ResB);
	ResH.makeStatusLine();
	ResH.integrate();
}


status_code_t	writeResponseBody(ResBody& ResB, const string& filepath)
{
	status_code_t	status = ResB.readFile(root + filepath);
	if (status == 404)
		ResB.readFile(root + "/404/404.html");

	return status;
}

void	writeResponseHeader(ResHeader& ResH, status_code_t status)
{
	ResH.setHTTPversion("HTTP/1.1");
	if (ResH.exist("status") == true)
	{
		pair<status_code_t, string>	sr = checkStatusField(ResH["Status"]);
		ResH.setStatusCode(sr.first);
		ResH.setReasonPhrase(sr.second);
		ResH.removeKey("Status");
	}
	else
	{
		ResH.setStatusCode(status);
		switch (ResH.getStatusCode())
		{
		case 200:	ResH.setReasonPhrase("OK");			break;
		case 404:	ResH.setReasonPhrase("Not Found");	break;
		/* and so on ... */
		}

	}
	ResH["Connection"]			= "close";
	ResH["Server"]				= "Webserv 0.1";
	// ResH["Keep-Alive"]			=
	// ResH["Last-Modified"]		=
	// ResH["E-Tag"]				=
	// ResH["Transfer-Encoding"]	=
}

//@------------------------make response header, body------------------------@//


//@---------------------------------make end---------------------------------@//
