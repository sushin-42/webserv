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
#include "ReqBody.hpp"
#include "ReqHeader.hpp"
#include "ResBody.hpp"
# include "color.hpp"
#include "httpError.hpp"

# include "Pipe.hpp"
# include "ResHeader.hpp"
# include "utils.hpp"
# include "Undone.hpp"

# define GET	1
# define PUT	2
# define POST	3
# define DELETE	4


/*
 @ There are 3 ways of detecting the end of the stream depending on what requests you are handling:
 * If it is a GET or HEAD request, you only need to read the HTTP headers, request body is normally ignored if it exists, so when you encounter \r\n\r\n, you reach the end of the request(actually the request headers).
 * If it is a POST method, read the Content-Length in the header and read up to Content-Length bytes.
 * If it is a POST method and the Content-Length header is absent, which is most likely to happen, read until -1 is returned, which is the signal of EOF.
*/


char checkMethod(const string& content);
class ServerSocket;
class ConnSocket : public ISocket
{
friend class ServerSocket;

/**========================================================================
* %                          member variables
*========================================================================**/

private:
	socklen_t			len;
	char				buf[1024];
	string				recvContent;	// cumulate received content

public:
	ReqHeader	ReqH;
	ReqBody		ReqB;
	ResHeader	ResH;
	ResBody		ResB;
	bool		pending;
	bool		chunk;		/* to distinguish script output chunk with server chunk */
	bool		FINsended;	/* we already sended FIN, DO NOT send more data. */

	Pipe*			linkInputPipe;
	Pipe*			linkOutputPipe;
	FileStream*		linkInputFile;
	FileStream*		linkOutputFile;
	ServerSocket*	linkServerSock;
	Config*			conf;


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

	void	recvRequest();

	void	send(const string& content, map<int, undone>& writeUndoneBuf);
	void	setErrorPage(status_code_t status, const string& reason, const string& text);
	void	returnError(status_code_t status, const string& message);

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

#endif
