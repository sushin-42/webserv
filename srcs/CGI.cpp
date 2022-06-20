//!   The script MUST return one of either
//.		a document response,
//*		a local redirect response
//'		a client redirect response (with optional document)

//@--------------------------------------------------------------------------@//
//@ CGI-Response = document-response | local-redir-response |			     @//
//@                client-redir-response | client-redirdoc-response          @//
//@--------------------------------------------------------------------------@//
# include "CGI.hpp"
# include "core.hpp"

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

		envs["GATEWAY_INTERFACE"] = "CGI/1.1",
		// envs["PATH_INFO"]	=,
		// envs["PATH_TRANSLATED"] = ,
		// envs["QUERY_STRING"]	= "" | query,
		envs["SERVER_PROTOCOL"] = connected->ReqH.getHTTPversion();
		envs["SERVER_SOFTWARE"] = "webserv";
		envs["SERVER_NAME"] = serv->getIP();
		envs["SERVER_PORT"] = serv->getPort();
		envs["REMOTE_ADDR"] = envs["REMOTE_HOST"] = connected->getIP();	// if host name exists, host == name
		// envs["AUTH_TYPE"] = auth-scheme ( Basic, Digest )
		// envs["REMOTE_USER"] = "",	// if AUTH_TYPE (Authorization )
		// envs["REMOTE_IDENT"] = ""	// if supports Ident protocol
		// envs["SCRIPT_NAME"] = "" | ( "/" path )
		// The SCRIPT_NAME variable MUST be set to a URI path (not URL-encoded) which could identify the CGI script,
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


int childRoutine(
				int PtoC[2],
				int CtoP[2],
				ServerSocket* serv,
				ConnSocket* connected
			)
{
	vector<char*> argv, envp;

	string				path = connected->conf->root + connected->ReqH.getRequsetTarget();
	argv.push_back(const_cast<char*>(path.c_str()));
	argv.push_back(NULL);

	map<string,string>				envm = makeCGIEnv(serv, connected);
	map<string,string>::iterator	it	= envm.begin();
	map<string,string>::iterator	ite	= envm.end();
	vector<string>					envps;
	for (; it != ite; it++)
		envps.push_back(it->first+ "=" + it->second);

	envp.reserve(envps.size() + 1);
	vector<string>::iterator		vit = envps.begin();
	vector<string>::iterator		vite = envps.end();
	for (; vit != vite; vit++)
		envp.push_back(const_cast<char*> (vit->c_str()));
	envp.push_back(NULL);

//#-----------------------------argv, envp done-----------------------------#//

	dup2(CtoP[1], STDOUT_FILENO), close(CtoP[0]), close(CtoP[1]);
	dup2(PtoC[0], STDIN_FILENO), close(PtoC[1]), close(PtoC[1]);
	// sleep(1);

	if (execve(
				(path).c_str(),
				(char * const*)(argv.data()),
				(char * const*)(envp.data())
			) == -1) {
				cerr << "exec error: " << strerror(errno) << errno <<endl;
			}
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
	close(PtoC[0]), close(CtoP[1]);

	string	ReqB = connected->ReqB.getContent();
	Pipe*	pr = new Pipe(CtoP[0], pid);
	Pipe*	pw = new Pipe(PtoC[1], pid);

	pr->linkConn = connected;
	connected->linkReadPipe = pr;
	pollset.enroll(pr);

	pw->linkConn = connected;
	connected->linkWritePipe = pw;
	PollSet::iterator it =  pollset.enroll(pw);
	it.first->events |= POLLOUT;
}

void	newProc(PollSet& pollset, ServerSocket* serv, ConnSocket* connected)
{
	int				PtoC[2], CtoP[2];
	pid_t			pid;

	pipe(CtoP), pipe(PtoC) ;
	fcntl(CtoP[0], F_SETFL, fcntl(CtoP[0], F_GETFL, 0) | O_NONBLOCK);

	pid = fork();
	if (pid == 0)	childRoutine(PtoC, CtoP, serv, connected);	//TODO: check return value -1
	else			parentRoutine(pollset, connected, pid, PtoC, CtoP);	// produce non-blocking pipe and poll.enroll(pipe)

	connected->pending = false;	//@ default == NO pending
}

void	moveToResH(const string& output, ConnSocket* connected)
{
	map<string,string>				tmp(KVtoMap(output, ':'));
	map<string,string>::iterator	it, ite;

	it = tmp.begin(), ite = tmp.end();
	for (; it != ite; it++)
	{
		if (it->first == lowerize("Content-type")||
			it->first == lowerize("Status")||
			it->first == lowerize("Location")||
			it->first == lowerize("Content-Length")||
			it->first == lowerize("Content-Range")||
			// it->first == lowerize("Transfer-Encoding")||
			it->first == lowerize("ETag")||
			connected->ResH.exist(it->first) == false	)

				connected->ResH[it->first] = it->second;
		else
				connected->ResH.append(it->first, it->second);
	}
}

void	processOutputHeader(PollSet& pollset, ServerSocket* serv, ConnSocket* connected, Pipe* CGIpipe)
{
	pair<status_code_t, string>		Status;

	moveToResH(CGIpipe->output, connected);

	if (connected->ResH.exist("Location"))
	{
		if (connected->ResH["Location"][0] == '/')	localRedir(pollset, serv, connected);
		else										clientRedir(connected);
	}
	else											documentResponse(connected);


	if (!connected->ResH.exist("Content-Length") &&
			(!connected->ResH.exist("Transfer-Encoding") ||
			lowerize(connected->ResH["Transfer-encoding"]) != "chunked"
		)
	)
	{
		connected->ResH.append("Transfer-encoding",  "chunked");
		connected->chunk = true;
		/**========================================================================
		 *'  			Server chunk. NOT same with script chunk
		 *	we encode(chunk) if and only if we set "Transfer-Encoding: chunked".
		 *	we DO NOT encode even if "Transfer-Encoding: chunked" exists in script output.
		 *	if we get output with it, body of the output will be send raw.
		 *	if script wants to be chunked, it MUST be chunked format by itself.
		 *
		 *========================================================================**/
	}

}


void	CGIRoutines(
					PollSet& pollset,
					ServerSocket* serv,
					ConnSocket* connected,
					Pipe* CGIpipe
				)
{
	ssize_t	byte = 0;
	if (!CGIpipe)
		newProc(pollset, serv, connected);
	else
	{

		switch (byte = CGIpipe->read())
		{

		case -1:	/* internal server error */
			TAG(CGI#, CGIroutines); cout << RED("Unexcpected error from pipe: ") << CGIpipe->getFD() << endl;
			CGIpipe->close();
			/* need to return 500! */
			break;

		case 0:		/* close pipe, process output */

			TAG(CGI#, CGIroutines); cout << GRAY("Pipe closed: ") << CGIpipe->getFD() << endl;
			CGIpipe->close();

			if (connected->pending == false)
			{
				//NOTE: if read 0 byte after n byte ?
				connected->ResB.setContent(
											connected->chunk ?
												makeChunk(CGIpipe->output) :
												CGIpipe->output
										);
			}
			connected->pending = false;
			break;

		default:	/* output appended */

			/* wait full header */
			if (CGIpipe->headerDone == false &&
					(CGIpipe->output.rfind("\r\n\r\n") != string::npos ||
					CGIpipe->output.rfind("\n\n") != string::npos))
			{
				processOutputHeader(pollset, serv, connected, CGIpipe);
				CGIpipe->headerDone = true;							// appended to conn->ResH
				CGIpipe->output = extractBody(CGIpipe->output);		// store remained after header
			}
			if (connected->pending == false)
			{
				connected->ResB.setContent(
											connected->chunk ?
												makeChunk(CGIpipe->output) :
												CGIpipe->output
										);
				CGIpipe->output.clear();
			}
		}
	}
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

void	localRedir(PollSet& pollset, ServerSocket* serv, ConnSocket* connected)
{
	if (connected->ResH.exist("Status") == false ||
			(connected->ResH.exist("Status") == true &&
			checkStatusField(connected->ResH["Status"]).first == 200
		)
	)
	{
		//@ regard as request to Location, but some header-fields from CGI remain @//
		//@ Content-Length, Content-Type, Transfer-Encoding will be replaced @//
		connected->pending = true;
		connected->ReqH.setRequsetTarget(connected->ResH["location"]);
		connected->ResH.removeKey("location");
		connected->ResH.removeKey("transfer-encoding");
		// connected->ResH.print();
		core(pollset, serv, connected);
	}

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
	if (connected->ResH.exist("Status") == false ||
			(connected->ResH.exist("Status") == true &&
			checkStatusField(connected->ResH["Status"]).first == 200
		)
	)

	{
		connected->pending = true;
		connected->ResH["Status"] = "302 Found";
		connected->ResB.setContent(
						errorpage(
								"302 Found",
								"Found",
								"<p>The document has moved <a href=\"" + connected->ResH["location"] + "\">here</a>.</p>"
							)
						);
		connected->ResH["Content-type"] = "text/html; charset=iso-8859-1";
		connected->ResH["Content-Length"] = toString(connected->ResB.getContent().length());
		connected->ResH.removeKey("Transfer-Encoding");

	}

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
}

