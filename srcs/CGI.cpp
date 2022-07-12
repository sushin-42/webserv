//!   The script MUST return one of either
//.		a document response,
//*		a local redirect response
//'		a client redirect response (with optional document)

//@--------------------------------------------------------------------------@//
//@ CGI-Response = document-response | local-redir-response |			     @//
//@                client-redir-response | client-redirdoc-response          @//
//@--------------------------------------------------------------------------@//
# include "CGI.hpp"
#include "Exceptions.hpp"
# include "core.hpp"
# include "ConfigChecker.hpp"
#include <unistd.h>

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
		envs["PATH_INFO"] = CHECK->getFileName(connected->conf, connected->ReqH.getURI().path),	//NOTE: absolute path or rel-path from location
		// envs["PATH_TRANSLATED"] = ,
		envs["QUERY_STRING"]	= connected->ReqH.getURI().query,
		envs["SERVER_PROTOCOL"] = connected->ReqH.getHTTPversion();
		envs["SERVER_SOFTWARE"] = WEBSERV_VERSION;
		envs["SERVER_NAME"] = connected->serverName;
		envs["SERVER_PORT"] = toString(serv->getPort());
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
				ConnSocket* connected,
				const string& executable,
				const string& scriptpath
			)
{
	vector<char*> argv, envp;


	argv.push_back(const_cast<char*>(executable.c_str()));
	argv.push_back(const_cast<char*>(scriptpath.c_str()));
	argv.push_back(NULL);

	// cerr << argv[0] << " | " << argv[1] << endl;
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
	dup2(PtoC[0], STDIN_FILENO), close(PtoC[0]), close(PtoC[1]);

	int devNull = open("/dev/null", O_WRONLY);
	if (devNull == -1)
	{
		cerr << "open \"/dev/null\" fail: " << strerror(errno) << endl;		//NOTE: INTERNAL SERVER ERROR
		exit(-1);
	}
	dup2(devNull, STDERR_FILENO);

	if (chdir(scriptpath.substr(0,scriptpath.find_last_of('/')).c_str()) == -1) {
		cerr << "chdir fail: " << strerror(errno) << endl;						//NOTE: INTERNAL SERVER ERROR
		exit(-1);
	}

	if (execve(
				(executable).c_str(),
				(char * const*)(argv.data()),
				(char * const*)(envp.data())
			) == -1) {
				cerr << "exec fail: " << strerror(errno) << endl;		//NOTE: INTERNAL SERVER ERROR
				exit(-1);
			}
	return -1;
}

void parentRoutine(
					int PtoC[2],
					int CtoP[2],
					ConnSocket* connected,
					pid_t pid
				)
{
	close(PtoC[0]), close(CtoP[1]);

	string	ReqB = connected->ReqB.getContent();
	Pipe*	pr = new Pipe(CtoP[0], pid);
	Pipe*	pw = new Pipe(PtoC[1], pid);

	pr->linkConn = connected;
	connected->linkInputPipe = pr;
	POLLSET->enroll(pr, POLLIN);

	pw->linkConn = connected;
	connected->linkOutputPipe = pw;
	POLLSET->enroll(pw, POLLOUT);
}

void	createCGI(ServerSocket* serv, ConnSocket* connected, const string& exe, const string& scriptpath)
{
	int				PtoC[2], CtoP[2];
	pid_t			pid;

	pipe(CtoP), pipe(PtoC) ;
	fcntl(CtoP[0], F_SETFL, O_NONBLOCK);
	fcntl(PtoC[1], F_SETFL, O_NONBLOCK);

	pid = fork();
	if (pid == 0)	childRoutine(PtoC, CtoP, serv, connected, exe, scriptpath);	//TODO: check return value -1
	else			parentRoutine(PtoC, CtoP, connected, pid);	// produce non-blocking pipe and poll.enroll(pipe)
}
