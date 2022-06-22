#include "CGI.hpp"
#include "Config.hpp"
#include "ConnSocket.hpp"
#include "Poll.hpp"
# include "ResBody.hpp"
# include "ResHeader.hpp"
#include "utils.hpp"
#include <string>

string			getFileName(ConnSocket* connected, const string& filepath);
// status_code_t	writeResponseBody(ConnSocket* connected, const string& filepath, PollSet& pollset);
void			writeResponseHeader(ConnSocket* connected);
void			core(PollSet& pollset, ServerSocket *serv, IStream *stream);
void			core_wrapper(PollSet& pollset, ServerSocket *serv, IStream* stream);

