#include "CGI.hpp"
#include "Config.hpp"
#include "ConnSocket.hpp"
#include "Poll.hpp"
# include "ResBody.hpp"
# include "ResHeader.hpp"
#include "utils.hpp"
#include <string>

string			getFileName(ConnSocket* connected, const string& filepath);
void			writeResponseHeader(ConnSocket* connected);
void			core(ServerSocket *serv, Stream *stream);
void			core_wrapper(ServerSocket *serv, Stream* stream);

