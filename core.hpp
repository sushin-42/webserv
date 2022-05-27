#include "ConnSocket.hpp"
# include "ResBody.hpp"
# include "ResHeader.hpp"

status_code_t	writeResponseBody(ResBody& ResB, const string& filepath);
void			writeResponseHeader(ResHeader& ResH, status_code_t status);

void	core(ResHeader& ResH, ResBody& ResB, const string& filepath)
{
	status_code_t status;
	status = writeResponseBody(ResB, filepath);
	ResH["Content-Type"]		= MIME[getExt(filepath)];
	ResH["Content-Length"]		= toString(ResB.getContent().length());
	/*---------------- exec CGI here ----------------*/
	//1.client-redir	: content-type and length set in localRedir()
	//2.document		: content-type and length set by script, if exists. else, APACHE: chunk LIGHTTPD: get length
	//3.local-redir		: content-type and length set by redir-page. but if script say Status : not 200  , it does not redirected. and ct, cl set by script.
	writeResponseHeader(ResH, status);
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
		// pair<status_code_t, string>	sr = checkStatusField(ResH["Status"]);
		// ResH.setStatusCode(sr.first);
		// ResH.setReasonPhrase(sr.second);
		// ResH.removeKey("Status");
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
	ResH.makeStatusLine();
	ResH.integrate();
}

//@------------------------make response header, body------------------------@//


//@---------------------------------make end---------------------------------@//
