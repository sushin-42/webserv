#include "CGI.hpp"
#include "Config.hpp"
#include "ConnSocket.hpp"
#include "Poll.hpp"
# include "ResBody.hpp"
# include "ResHeader.hpp"
#include "utils.hpp"

status_code_t	writeResponseBody(ConnSocket* connected, const string& filepath);
void			writeResponseHeader(ConnSocket* connected);
void			core(PollSet& pollset, ServerSocket *serv, ConnSocket *connected);
void			core_wrapper(PollSet& pollset, ServerSocket *serv, ConnSocket *connected, Pipe* CGIpipe);

