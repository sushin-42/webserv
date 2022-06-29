//!   The script MUST return one of either
//.		a document response,
//*		a local redirect response
//'		a client redirect response (with optional document)

//@--------------------------------------------------------------------------@//
//@ CGI-Response = document-response | local-redir-response |			     @//
//@                client-redir-response | client-redirdoc-response          @//
//@--------------------------------------------------------------------------@//

#ifndef CGI_HPP
# define CGI_HPP

# include <_ctype.h>
# include <cstddef>
# include <cstring>
# include <string>
# include <sys/_types/_ssize_t.h>
# include <sys/fcntl.h>
# include <sys/wait.h>
# include <utility>
# include <vector>

# include "ConnSocket.hpp"
# include "Poll.hpp"
# include "ServerSocket.hpp"
# include "ReqBody.hpp"
# include "ReqHeader.hpp"
# include "ResBody.hpp"
# include "ResHeader.hpp"
// # include "utils.hpp"
using namespace std;

pair<status_code_t, string>	checkStatusField(const string& status);
string	toMetaVar(const string& s, string scheme);
map<string, string>	makeCGIEnv(ServerSocket* serv, ConnSocket* connected);


int childRoutine(
				int PtoC[2],
				int CtoP[2],
				ServerSocket* serv,
				ConnSocket* connected,
				const string& executable,
				const string& scriptpath
			);

void parentRoutine(
					int PtoC[2],
					int CtoP[2],
					ConnSocket* connected,
					pid_t pid
				);

void	createCGI(ServerSocket* serv, ConnSocket* connected, const string& executable, const string& scriptpath);



#endif
