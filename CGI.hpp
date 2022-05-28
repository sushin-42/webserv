//!   The script MUST return one of either
//.		a document response,
//*		a local redirect response
//'		a client redirect response (with optional document)

//@--------------------------------------------------------------------------@//
//@ CGI-Response = document-response | local-redir-response |			     @//
//@                client-redir-response | client-redirdoc-response          @//
//@--------------------------------------------------------------------------@//




//#   script-URI = <scheme> "://" <server-name> ":" <server-port>
//#  				<script-path> <extra-path> "?" <query-string>
//#
//#   where <scheme> is found from SERVER_PROTOCOL, <server-name>,
//#   <server-port> and <query-string> are the values of the respective
//#   meta-variables.  The SCRIPT_NAME and PATH_INFO values, URL-encoded
//#   with ";", "=" and "?"  reserved, give <script-path> and <extra-path>.

// http://localhost:8080/test.py/additional/path
// REQUEST_URI	=> /test.py/additional/path
// SCRIPT_NAME => /test.py
// PATH_INFO => /additional/path

// PATH_TRANSLATED => /Users/shin/Desktop/Git/webserv/static_file/additional/path
// QUERY_STRING
// SCRIPT_FILENAME => /Users/shin/Desktop/Git/webserv/static_file/test.py

#ifndef CGI_HPP
# define CGI_HPP

#include <_ctype.h>
# include <cstddef>
# include <cstring>
# include <string>
// #include <sys/_types/_pid_t.h>
#include <sys/_types/_ssize_t.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
# include <vector>

#include "ConnSocket.hpp"
#include "Poll.hpp"
# include "ServerSocket.hpp"
# include "ReqBody.hpp"
# include "ReqHeader.hpp"
# include "ResBody.hpp"
# include "ResHeader.hpp"
# include "utils.hpp"
using namespace std;

void	documentResponse(ConnSocket* connected);
void	clientRedir(ConnSocket* connected);
void	localRedir(PollSet& pollset, ServerSocket* serv, ConnSocket* connected);
void	core(PollSet& pollset, ServerSocket *serv, ConnSocket *connected);

pair<status_code_t, string>	checkStatusField(const string& status)
{
	status_code_t	statusCode = 502;
	string			mayStatusCode;
	string			reasonPhrase = "";

	string::size_type posSP = status.find(' ');
	mayStatusCode = status.substr(0, posSP);
	mayStatusCode = ltrim(mayStatusCode, " \t");

	if (mayStatusCode.length() == 3)
	{
		if (	isdigit(mayStatusCode[0]) &&
				isdigit(mayStatusCode[1]) &&
				isdigit(mayStatusCode[2])	)
			statusCode = atoi(mayStatusCode.c_str());

		string::size_type posTEXT = status.find_first_not_of(" \r\v\f\t", posSP);
		if	(posTEXT != string::npos)
			reasonPhrase = status.substr(posTEXT);
	}
	return make_pair(statusCode, reasonPhrase);
}
string	toMetaVar(const string& s, string scheme)
{
	string				ret(scheme + "_" + s);
	string::iterator	it;
	string::iterator	ite = ret.end();

	for (it = ret.begin(); it < ite; it++)
	{
		if (islower(*it))
			*it = toupper(*it);
		else if (*it == '-')
			*it = '_';
	}
	return ret;
}

map<string, string>	makeCGIEnv(ServerSocket* serv, ConnSocket* connected)
{
		map<string, string>				envs;
		map<string, string>				hf = connected->ReqH.getHeaderField();
		map<string, string>::iterator	it = hf.begin();
		map<string, string>::iterator	ite = hf.end();
		envs["REQUEST_METHOD"] = connected->ReqH.getMethod();
		if (!connected->ReqB.getContent().empty())
		{
			envs["CONTENT_TYPE"] = connected->ReqH["content-type"];
			envs["CONTENT_LENGTH"] = toString(connected->ReqB.getContent().length());
		}
		// if body exist, MUST, else NULL (after the server has removed any transfer-codings or content-codings.)
		// The server MUST set this meta-variable if an HTTP Content-Type field is present in the client request header.
		// If the server receives a request with an attached entity but no Content-Type header field,
		// it MAY attempt to determine the correct content type, otherwise it should omit this meta-variable.

		envs["GATEWAY_INTERFACE"] = "CGI/1.1",
		// "PATH_INFO" =,
		// "PATH_TRANSLATED" = ,
		// "QUERY_STRING" = "" | query,

		envs["SERVER_PROTOCOL"] = connected->ReqH.getHTTPversion();
		envs["SERVER_SOFTWARE"] = "webserv";
		envs["SERVER_NAME"] = serv->getIP();
		envs["SERVER_PORT"] = serv->getPort();
		envs["REMOTE_ADDR"] = envs["REMOTE_HOST"] = connected->getIP();	// if host name exists, host == name


		// envs["AUTH_TYPE"] = auth-scheme ( Basic, Digest )
		// envs["REMOTE_USER"] = "",	// if AUTH_TYPE (Authorization )
		// envs["REMOTE_IDENT"] = ""	// if supports Ident protocol

		// envs["SCRIPT_NAME"] = ""
		// The SCRIPT_NAME variable MUST be set to a URI path (not URL-encoded) which could identify the CGI script,
		//  SCRIPT_NAME = "" | ( "/" path )
   		// The leading "/" is not part of the path.
		// It is optional if the path is NULL; however, the variable MUST still be set in that case.

		for (; it != ite; it++)
		{
			if (it->first == "content-type" ||
				it->first == "content-length")	continue;
			string t = toMetaVar(it->first, "HTTP");
			envs[t] = it->second;
		}
		//If multiple header fields with the same field-name
   		//are received then the server MUST rewrite them as a single value
   		//having the same semantics.
   		//Similarly, a header field that spans multiple lines MUST be merged onto a single line.
		//In particular, it SHOULD remove any header fields carrying authentication information, such as
   		// 'Authorization'; or that are available to the script in other
   		// variables, such as 'Content-Length' and 'Content-Type'.
   		// The server MAY remove header fields that relate solely to client-side
   		// communication issues, such as 'Connection'.
		return envs;
};


pid_t	forkCGI(
				ServerSocket* serv, ConnSocket* connected,
				vector<char *>& argv, vector<char *>& envp
			)
{
	string							path = connected->ReqH.getRequsetTarget();
	map<string,string>				envm = makeCGIEnv(serv, connected);
	map<string,string>::iterator	it	= envm.begin();
	map<string,string>::iterator	ite	= envm.end();

	vector<string> envps;
	vector<string>::iterator vit;
	vector<string>::iterator vite;
	for (; it != ite; it++)
		envps.push_back(it->first+ "=" + it->second);

	envp.reserve(envps.size() + 1);
	vit=envps.begin(), vite=envps.end();
	for (; vit != vite; vit++)
		envp.push_back(const_cast<char*> (vit->c_str()));
	envp.push_back(NULL);

	argv.push_back(const_cast<char*>(path.data()));
	argv.push_back(NULL);

	return fork();
}

int childRoutine(
				int PtoC[2],
				int CtoP[2],
				const string& path,
				vector<char*> argv,
				vector<char*> envp
			)
{
	dup2(CtoP[1], STDOUT_FILENO), close(CtoP[0]), close(CtoP[1]);
	dup2(PtoC[0], STDIN_FILENO), close(PtoC[1]), close(PtoC[1]);
	sleep(1);
	if (execve(
				(root+path).c_str(),
				(char * const*)(argv.data()),
				(char * const*)(envp.data())
			) == -1)	cerr << "exec error: " << strerror(errno) << errno <<endl;
	return -1;
}

void parentRoutine(
					PollSet& pollset,
					ConnSocket* connected,
					pid_t pid,
					int PtoC[2],
					int CtoP[2]
				)
{

		string	ReqB = connected->ReqB.getContent();
		Pipe*	p = new Pipe(CtoP[0], pid);
		p->linkConn = connected;

		close(PtoC[0]), close(CtoP[1]);
		write(PtoC[1], ReqB.c_str(), ReqB.length());
		close(PtoC[1]);

		pollset.enroll(p);
		// waitpid(p->pid, &p->status, WNOHANG);
}

void	newProc(PollSet& pollset, ServerSocket* serv, ConnSocket* connected)
{
	int				PtoC[2], CtoP[2];
	pid_t			pid;
	vector<char*>	argv;
	vector<char*>	envp;

	pipe(PtoC), pipe(CtoP);
	fcntl(CtoP[0], F_SETFL, fcntl(CtoP[0], F_GETFL, 0) | O_NONBLOCK);

	pid = forkCGI(serv, connected, argv, envp);
	if (pid == 0)	childRoutine(PtoC, CtoP, connected->ReqH.getRequsetTarget(), argv, envp);	//TODO: check return value -1
	else			parentRoutine(pollset, connected, pid, PtoC, CtoP);			// produce non-blocking pipe and poll.enroll(pipe)
}


void	processOutput(PollSet& pollset, ServerSocket* serv, ConnSocket* connected, Pipe* CGIpipe)
{
	map<string,string>				tmp(KVtoMap(CGIpipe->output, ':'));
	map<string,string>::iterator	it, ite;
	it = tmp.begin(), ite = tmp.end();
	for (; it != ite; it++)
	{
		if (it->first == lowerize("Content-type")||
			it->first == lowerize("Status")||
			it->first == lowerize("Location")||
			it->first == lowerize("Content-Length")||
			it->first == lowerize("Content-Range")||
			it->first == lowerize("Transfer-Encoding")||
			it->first == lowerize("ETag")||
			connected->ResH.exist(it->first) == false	)

				connected->ResH[it->first] = it->second;

		else
				connected->ResH.append(it->first, it->second);
	}

	connected->ResB.setContent(extractBody(CGIpipe->output));
	if (connected->ResH.exist("location"))
	{
		if (connected->ResH["location"][0] == '/')	localRedir(pollset, serv, connected);
		else										clientRedir(connected);
	}
	else											documentResponse(connected);
}

void	CGIRoutines(
					PollSet& pollset,
					ServerSocket* serv,
					ConnSocket* connected,
					Pipe* CGIpipe
				)
{
	ssize_t	byte = 0;
	connected->pending = true;
	if (!CGIpipe)
		newProc(pollset, serv, connected);
	else
	{
		byte = CGIpipe->read();

		if		(byte == -1)	// output appended, go back main loop,
		{
			return;
		}

		else if (byte == 0)		// close pipe, process output
		{
			TAG(CGI#, CGIroutines()); cout << GRAY("Pipe closed: ") << CGIpipe->getFD() << endl;
			CGIpipe->close();
			CGIpipe->setFD(-1);
			processOutput(pollset, serv, connected, CGIpipe);
		}

	}

	//1.client-redir	: if no Status -> set 302, Found	//@
	//					  if Content-length not in script, set in clientRedir() //@
	//            		  if CL in script and not matched with body, timeout occur (see LIGHTTPD / what if APACHE?) //!

	//2.document		: Content-length set by script, if exists. //@
	//		   			  else, APACHE: chunk-encoding
	// 							LIGHTTPD: get length of CGI body //@
	// 					  if CL in script not matched with body, timeout occur (see LIGHTTPD / what if APACHE?) //!

	//3.local-redir		: if (Status := 200 | "" ) Content-Length, Type are newly set by local-redir-page. Location removed.
}




//*--------------------------------------------------------------------------*//
//* local-redir-response = local-Location NL                                 *//
//*                                                                          *//
//*  The CGI script can return a URI path and query-string                   *//
//*	 ('local-pathquery') for a local resource in a Location header field.    *//
//*  This indicates to the server that it should reprocess the request       *//
//*  using the path specified.                                               *//
//*                                                                          *//
//*  The script MUST NOT return any other header fields or a message-body,   *//
//*  and the server MUST generate the response that it would have produced   *//
//*  in response to a request containing the URL                             *//
//*                                                                          *//
//*  scheme "://" server-name ":" server-port local-pathquery                *//
//*--------------------------------------------------------------------------*//


//3.local-redir		: if (Status := 200 | None ) Content-Length, Type are newly set by local-redir-page. Location removed.
//					  else:	 no redirection. if no redir, No header modification.
void	localRedir(PollSet& pollset, ServerSocket* serv, ConnSocket* connected)
{
	if (connected->ResH.exist("Status") &&
		checkStatusField(connected->ResH["Status"]).first != 200)
		return ;

	//@ regard as request to Location, but some header-fields from CGI remain @//
	connected->ReqH.setRequsetTarget(connected->ResH["location"]);
	connected->ResH.removeKey("location");

	core(pollset, serv, connected);
}


//'--------------------------------------------------------------------------'//
//' client-redir-response = client-Location *extension-field NL              '//
//'                                                                          '//
//'   The CGI script can return an absolute URI path in a Location header    '//
//'	 field, to indicate to the client that it should reprocess the request   '//
//'  using the URI specified.                                                '//
//'                                                                          '//
//'   The script MUST not provide any other header fields, except for        '//
//'  server-defined CGI extension fields.  For an HTTP client request, the   '//
//'  server MUST generate a 302 'Found' HTTP response message.               '//
//'--------------------------------------------------------------------------'//

//%--------------------------------------------------------------------------%//
//% client-redirdoc-response = client-Location Status Content-Type           %//
//%                            *other-field NL response-body                 %//
//%                                                                          %//
//%   The CGI script can return an absolute URI path in a Location header    %//
//%	 field together with an attached document, to indicate to the client     %//
//%  that it should reprocess the request using the URI specified.           %//
//%                                                                          %//
//%   The Status header field MUST be supplied and MUST contain a status     %//
//%  value of 302 'Found', or it MAY contain an extension-code, that is,     %//
//%  another valid status code that means client redirection.  The server    %//
//%  MUST make any appropriate modifications to the script's output to       %//
//%  ensure that the response to the client complies with the response       %//
//%  protocol version.                                                       %//
//%--------------------------------------------------------------------------%//

void	clientRedir(ConnSocket* connected)		//check 303
{
	if (!connected->ResH.exist("Status"))
	{
		connected->ResH["Status"] = "302 Found";
		connected->ResB.setContent(
						errorpage(
								"302 Found",
								"Found",
								"<p>The document has moved <a href=\"" + connected->ResH["location"] + "\">here</a>.</p>"
							)
						);
		connected->ResH["Content-type"] = "text/html; charset=iso-8859-1";
	}
	if (!connected->ResH.exist("Content-Length"))
		connected->ResH["Content-Length"] = toString(connected->ResB.getContent().length());

	// set timeout for case that not matched CL

	// if (ResH.getStatusCode() != 206 || ResH.getStatusCode() != 416)
		// erase Content-Range
}


//.--------------------------------------------------------------------------.//
//. document-response = Content-Type [ Status ] *other-field NL              .//
//.                          response-body                                   .//
//.                                                                          .//
//.  The script MUST return a Content-Type header field.                     .//
//.	 A Status header field is optional, and status 200 'OK' if omitted.      .//
//.  The server MUST make any appropriate modifications to the script output .//
//.  to ensure that the response to the client                               .//
//.  complies with the response protocol version.                            .//
//.--------------------------------------------------------------------------.//

void	documentResponse(ConnSocket* connected)
{
	if (!connected->ResH.exist("Status"))
		connected->ResH["Status"] = "200 OK";
	if (!connected->ResH.exist("Content-length"))
		connected->ResH["Content-Length"] = toString(connected->ResB.getContent().length());

	/*TODO*/
	//	content-length would not be removed.
	//	if given content-length by script is not accurate, browser wait infinitely.
	//	need to handle.

}
#endif
