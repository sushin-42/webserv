/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mishin <mishin@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/24 14:47:48 by mishin            #+#    #+#             */
/*   Updated: 2022/05/24 15:33:57 by mishin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
# define CGI_HPP



# include <cstddef>
# include <cstring>
# include <string>
# include <vector>
# include "ReqBody.hpp"
# include "ReqHeader.hpp"
# include "ResBody.hpp"
# include "ResHeader.hpp"
# include "utils.hpp"
using namespace std;

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

map<string, string>	makeCGIEnv(const ReqHeader& ReqH, const ReqBody& ReqB)
{
		map<string, string>				envs;
		map<string, string>				hf = ReqH.getHeaderField();
		map<string, string>::iterator	it = hf.begin();
		map<string, string>::iterator	ite = hf.end();
		envs["REQUEST_METHOD"] = ReqH.getMethod();
		envs["AUTH_TYPE"] = ""; // | auth-scheme
		if (ReqB.getContent() == "")
			cout << "NO BODY" << endl;
		else
		{
			envs["CONTENT_TYPE"] = ReqH["content-type"];
			envs["CONTENT_LENGTH"] = toString(ReqB.getContent().length());
		}
		//if body MUST, else NULL (after the server has removed any transfer-codings or content-codings.)
		// The server MUST set this meta-variable if an HTTP Content-Type field is present in the client request header.
		// If the server receives a request with an attached entity but no Content-Type header field,
		// it MAY attempt to determine the correct content type, otherwise it should omit this meta-variable.

		envs["GATEWAY_INTERFACE"] = "CGI/1.1",
		// "PATH_INFO" =,
		// "PATH_TRANSLATED" = ,
		// "QUERY_STRING" = "" | query,

		// "REMOTE_ADDR" = client-ip,					//! NEED TO REFACTOR POLLSET. NEED TO KNOW CLIENT SOCK INFO.
		// "REMOTE_HOST" = if not exist, client-ip
		// envs["REMOTE_USER"] = "",
		// envs["REMOTE_IDENT"] = "",
		envs["SERVER_PROTOCOL"] = ReqH.getHTTPversion();
		envs["SERVER_SOFTWARE"] = "webserv";

		// "SCRIPT_NAME" = ""
		//The SCRIPT_NAME variable MUST be set to a URI path (not URL-encoded) which could identify the CGI script,
		//       SCRIPT_NAME = "" | ( "/" path )
   		// The leading "/" is not part of the path.  It is optional if the path is NULL; however, the variable MUST still be set in that case.

		// "SERVER_NAME" = hostname | ipv4-address | ( "[" ipv6-address "]" ),
		// "SERVER_PORT" = port number,
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


//@ CGI routines
void	CGIRoutines(
					const ReqHeader& ReqH,
					const ReqBody& ReqB,
					ResHeader& ResH,
					ResBody& ResB
				)
{

		pid_t							pid;
		int								PtoC[2], CtoP[2];
		string							path = ReqH.getRequsetTarget();
		map<string,string>				envm = makeCGIEnv(ReqH, ReqB);
		map<string,string>::iterator	it	= envm.begin();
		map<string,string>::iterator	ite	= envm.end();


		vector<char*> argv;
		vector<char*> envp;
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

		pipe(PtoC);
		pipe(CtoP);

		argv.push_back(const_cast<char*>(path.data()));
		argv.push_back(NULL);

		pid = fork();
		if (pid == 0)	//. Child
		{
			dup2(CtoP[1], STDOUT_FILENO), close(CtoP[0]), close(CtoP[1]);
			dup2(PtoC[0], STDIN_FILENO), close(PtoC[1]), close(PtoC[1]);
			sleep(1);
			if (execve(
						(root+path).c_str(),
						(char * const*)(argv.data()),
						(char * const*)(envp.data())
					) == -1)	cerr << "exec error: " << strerror(errno) << errno <<endl;
		}
		else		//%	Parent
		{
			// read header & body from forked process
			close(PtoC[0]);
			close(CtoP[1]);
			int stat = 0;
			write(PtoC[1], ReqB.getContent().c_str(), ReqB.getContent().length());
			close(PtoC[1]);
			waitpid(pid, &stat, 0);

			string	content;
			ssize_t byte = readFrom(CtoP[0], content);
			cout << byte << endl;
			cout << content << endl;
			map<string, string>	tmp	= KVtoMap(content, ':');

			it = tmp.begin(), ite = tmp.end();
			for (; it != ite; it++)
				ResH[it->first] = it->second;
			ResB.setContent(content.substr(content.find("\n\n") + 1));

			close(CtoP[0]), close(CtoP[1]);
			close(PtoC[0]), close(PtoC[1]);

		}
}




#endif
