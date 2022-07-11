#ifndef CONNSOCKET_HPP
# define CONNSOCKET_HPP

#include <cstring>
#include <exception>
# include <iostream>
#include <string>
#include <sys/_types/_ssize_t.h>
#include <sys/errno.h>
#include <sys/poll.h>
#include <sys/wait.h>
#include <typeinfo>
# include <utility>

# include <sys/fcntl.h>

#include "FileStream.hpp"
# include "ISocket.hpp"
# include "Config.hpp"
#include "Stream.hpp"
#include "ReqBody.hpp"
#include "ReqHeader.hpp"
#include "ResBody.hpp"
# include "color.hpp"
#include "HTTP_Error.hpp"

# include "Pipe.hpp"
# include "ResHeader.hpp"
# include "utils.hpp"


# define GET	1
# define PUT	2
# define POST	3
# define DELETE	4
# define MAX_INTERNAL_REDIRECT 10

/*
 @ There are 3 ways of detecting the end of the stream depending on what requests you are handling:
 * If it is a GET or HEAD request, you only need to read the HTTP headers, request body is normally ignored if it exists, so when you encounter \r\n\r\n, you reach the end of the request(actually the request headers).
 * If it is a POST method, read the Content-Length in the header and read up to Content-Length bytes.
 * If it is a POST method and the Content-Length header is absent, which is most likely to happen, read until -1 is returned, which is the signal of EOF.
*/


class ServerSocket;
class ConnSocket : public ISocket
{
friend class ServerSocket;

/**========================================================================
* %                          member variables
*========================================================================**/

private:
	socklen_t			len;

public:
	string				recvContent;	// cumulate received content
	ReqHeader			ReqH;
	ReqBody				ReqB;
	ResHeader			ResH;
	ResBody				ResB;
	bool				pending;
	bool				chunk;				/* to distinguish script output chunk with server chunk */
	bool				FINsended;			/* we already sended FIN, DO NOT send more data. */
	bool				internalRedirect;
	bool				keepAlive;
	int					internalRedirectCount;
	int					currentReqCount;

	Pipe*				linkInputPipe;
	Pipe*				linkOutputPipe;
	FileStream*			linkInputFile;
	FileStream*			linkOutputFile;
	ServerSocket*		linkServerSock;
	Config*				conf;
	string				serverName;


/**========================================================================
* @                           Constructors
*========================================================================**/

public:
	ConnSocket();
	~ConnSocket();

/**========================================================================
* *                            operators
*========================================================================**/

	ConnSocket&	operator=( const ConnSocket& src );

/**========================================================================
* #                          member functions
*========================================================================**/

	bool	isPipeAlive();

	void	gracefulClose();
	void	setHeaderOrReadMore();
	void	setBodyOrReadMore();

	void	unlink(Stream* link);
	void	unlinkAll();


	void	makeResponseHeader();
	void	setErrorPage(status_code_t status, const string& reason, const string& text);
	void	returnError(HTTP_Error& error);
	void	checkErrorPage();

	void	recv();
	void	coreDone();
	void	send(const string& content);
	void	core();
	string	getOutputContent();

/**========================================================================
* !                            Exceptions
*========================================================================**/
class connClosed: public exception
{
	private:	string msg;
	public:		explicit connClosed();
				explicit connClosed(const string& m);
				virtual ~connClosed() throw();
				virtual const char * what() const throw();
};
private:
	void	dummy();

};


bool						isDynamicResource(Config* conf, const string& filename);
void						writePUToutputFile(ConnSocket* c, const string& fn);

pair<status_code_t, string>	controlFile(const string& method, ConnSocket* connected, const string& filename); /* wrapper for PUT | DELETE */
pair<status_code_t, string>	createPUToutputFile(ConnSocket* connected, const string filename);
pair<status_code_t, string>	deleteFile(const string& filename);

#endif
