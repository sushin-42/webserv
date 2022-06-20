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
				ConnSocket* connected
			);

void parentRoutine(
					PollSet& pollset,
					ConnSocket* connected,
					pid_t pid,
					int PtoC[2],
					int CtoP[2]
				);

void	newProc(PollSet& pollset, ServerSocket* serv, ConnSocket* connected);
void	moveToResH(const string& output, ConnSocket* connected);

void	processOutputHeader(PollSet& pollset, ServerSocket* serv, ConnSocket* connected, Pipe* CGIpipe);
void	CGIRoutines(
					PollSet& pollset,
					ServerSocket* serv,
					ConnSocket* connected,
					Pipe* CGIpipe
				);


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

void	localRedir(PollSet& pollset, ServerSocket* serv, ConnSocket* connected);


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

void	clientRedir(ConnSocket* connected);		//check 303


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

void	documentResponse(ConnSocket* connected);
#endif
