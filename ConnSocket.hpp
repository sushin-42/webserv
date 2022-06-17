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

# include "ISocket.hpp"
# include "Config.hpp"
#include "ReqBody.hpp"
#include "ReqHeader.hpp"
#include "ResBody.hpp"
# include "color.hpp"

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

	Pipe*		linkReadPipe;
	Pipe*		linkWritePipe;


/**========================================================================
* @                           Constructors
*========================================================================**/

public:
	ConnSocket()
	: ISocket(), len(sizeof(info)), recvContent(), ReqH(), ReqB(), ResH(), ResB(), pending(false), chunk(false), FINsended(false), linkReadPipe(NULL), linkWritePipe(NULL) {}
	~ConnSocket() {}

/**========================================================================
* *                            operators
*========================================================================**/

	ConnSocket&	operator=( const ConnSocket& src )
	{
		if (this != &src)
		{
			this->ISocket::operator=(src);
			this->ReqH		= src.ReqH;
			this->ReqB		= src.ReqB;
			this->ResH		= src.ResH;
			this->ResB		= src.ResB;
			this->pending	= src.pending;
			this->chunk		= src.chunk;
			this->FINsended	= src.FINsended;
			this->linkReadPipe	= src.linkReadPipe;
			this->linkWritePipe	= src.linkWritePipe;
		}
		return *this;
	}

/**========================================================================
* #                          member functions
*========================================================================**/

	bool	isPipeAlive()
	{
		pid_t	pid	= 0;

		pid = waitpid(linkReadPipe->pid, &linkReadPipe->status, WNOHANG);
		if (!(pid == linkReadPipe->pid || pid == -1))
		{
			TAG(ConnSocket, isPipeAlive) <<  _NOTE(pipe still alive: ) <<  _UL << linkReadPipe->pid << _NC << endl;
			return true;
		}
		else
		{
			TAG(ConnSocket, isPipeAlive) << _GOOD(waitpid on ) << linkReadPipe->pid << CYAN(" returns ") << _UL << pid << _NC << endl;
			return false;
		}
	}

	void	gracefulClose()
	{
		//NOTE: what if client doesn't send FIN? now we send FIN and close "after client send FIN too".
		// Have to close() instantly after send FIN, with short timer.
		shutdown(this->fd, SHUT_WR);
		/*
			IMPL: if lingering_time, last_active = now, this->setTimeOut(time_t to)
			need to move FIN sended, we do not any processing
		*/

		/*
			for case that client keep sending message even after FIN.
			we prevent calling ConnSocket#send().
		*/
		FINsended = true;

		TAG(ConnSocket, gracefulClose) << _GOOD(server send FIN: ) << _UL << this->fd << _NC << endl;
	}

	void	setHeaderOrReadMore()
	{
		if (has2CRLF(recvContent))	//NOTE: what if bad-format request doesn't contain "\r\n\r\n"?
		{
			//IMPL: keep-alive request count--
			if (isValidHeader(recvContent))
			{
				/* set ReqH here */
				switch (checkMethod(recvContent))
				{
				case GET:	ReqH.setMethod("GET");		break;
				case PUT:	ReqH.setMethod("PUT");		break;
				case POST:	ReqH.setMethod("POST");		break;
				case DELETE:ReqH.setMethod("DELETE");	break;
				default: throw methodNotAllowed();
				}
				ReqH.setHTTPversion("HTTP/1.1");	//TODO: parse from request
				ReqH.setRequsetTarget(recvContent);
				ReqH.setContent(extractHeader(recvContent));
				ReqH.setHeaderField(KVtoMap(recvContent, ':'));

				/* extract trailing body */
				recvContent = extractBody(recvContent);
			}
			else
				throw badRequest();
		}
		else
			throw readMore();
	}

	void	setBodyOrReadMore()
	{
		if (ReqH.exist("Transfer-Encoding"))	// it will override Content-Length
		{
			/*
				current:
				- discard after "0\r\n\r\n" ?
				- accept payload contains "0\r\n\r\n"
			*/
			ReqB.setChunk(recvContent);
			try
			{
				ReqB.decodingChunk();
				cout << "------------" << endl;
				cout << ReqB.getContent() << endl;
				cout << "------------" << endl;
			}
			catch (exception& e)
			{
				if (CONVERT(&e, ReqBody::invalidChunk))
					throw badRequest();
				if (CONVERT(&e, ReqBody::readMore) ||
					CONVERT(&e, ConnSocket::readMore))
					throw readMore();
			}

		}
		else if (ReqH.exist("Content-Length"))
		{
			if (!isNumber(ReqH["Content-Length"]))
				throw badRequest();
			//IMPL: compare "Content-Length" with client_max_body_size
			if (toNum<unsigned int>(ReqH["Content-Length"]) <= recvContent.length())
			{
				ReqB.setContent(recvContent.substr(0, toNum<unsigned int>(ReqH["Content-Length"])));
			}
			else
				throw readMore();
			/*
				if invalid content-length,
				send 400(Bad request), close connection.	// @
			*/
			/*
				if timeout before read all content-length,
				send 408(Request timeout), close connection.
			*/
		}
		else
		{
			/*
				if got body without length
				send 411(Length Required)
			*/
		}
	}

	void	recvRequest()
	{
		ssize_t		byte	= 0;
		bzero(buf, sizeof(buf));
		byte = read(this->fd, this->buf, sizeof(buf));

		switch (byte)
		{
		case 0:
			{
				TAG(ConnSocket, recvRequest); cout << GRAY("CLIENT EXIT ") << this->fd << endl;
				if (ReqH.exist("Content-Length") &&
					toNum<unsigned int>(ReqH["Content-Length"]) > recvContent.length())
				{
					/*
						if connection closed before get all content-length,
						send 400(Bad request), close connection.
					*/
					throw badRequest();
				}
				throw connClosed();
			}
		case -1:
			{
				if (errno != EAGAIN && errno != EWOULDBLOCK)
					throw somethingWrong(strerror(errno));
				else
					TAG(ConnSocket, recvRequest) << YELLOW("No data to read") << endl;
				break;
			}
		default:
			recvContent.append(buf, byte);
		}

		if (ReqH.empty())
		{
			try						{ setHeaderOrReadMore(); }
			catch (exception& e)	{ throw;}
		}

		if (!ReqH.empty())
		{
			try						{ setBodyOrReadMore(); }
			catch (exception& e)	{ throw; }
		}

	}

	void	send(const string& content, map<int, undone>& writeUndoneBuf)
	{
		if (FINsended) return;
		try						{ writeUndoneBuf.at(this->fd); }
		catch (exception& e)	{ writeUndoneBuf[this->fd] = (struct undone){"",0};
								  writeUndoneBuf[this->fd].content.append(content.data(), content.length());	}

		string&		rContent	= writeUndoneBuf[this->fd].content;
		ssize_t&	rWrited		= writeUndoneBuf[this->fd].totalWrited;
		ssize_t		rContentLen	= rContent.length();
		ssize_t		byte		= 0;

		byte = write( this->fd,
					  rContent.data() + rWrited,
					  rContentLen - rWrited );
		if (byte > 0)
			rWrited += byte;


		//@ all data sended @//
		if (rWrited == rContentLen)
		{
			TAG(ConnSocket, send) << _GOOD(all data sended to) << this->fd << ": " << rWrited << " / " << rContentLen << " bytes" << endl;
			writeUndoneBuf.erase(this->fd);
			if (linkReadPipe && isPipeAlive())
				throw sendMore();
			else
				gracefulClose();
		}
		//' not all data sended. have to be buffered '//
		else
		{
			TAG(ConnSocket, send) << _NOTE(Not all data sended to) << this->fd << ": " << rWrited << " / " << rContentLen  << " bytes" << endl;
			if (byte == -1)
			{
				TAG(ConnSocket, send) << _FAIL(unexpected error: ) << errno << endl;
				writeUndoneBuf.erase(this->fd);
				gracefulClose();
				throw exception();	// close and Drop now!
			}
			throw sendMore();
		}
	}


	void	setErrorPage(status_code_t status, const string& reason, const string& text)
	{
		this->ResH.setHTTPversion("HTTP/1.1");
		this->ResH.setStatusCode(status);
		this->ResH.setReasonPhrase(reason);
		this->ResB.setContent(
							errorpage(
									toString(status) + " " + reason,
									reason,
									text
								)
							);
		this->ResH["Content-type"] = "text/html; charset=iso-8859-1";
		this->ResH["Content-Length"] = toString(this->ResB.getContent().length());
	}

/**========================================================================
* !                            Exceptions
*========================================================================**/
	class connClosed: public exception
	{
		private:	string msg;
		public:		explicit connClosed(): msg("") {}
					explicit connClosed(const string& m): msg(m) {}
					virtual ~connClosed() throw() {};
					virtual const char * what() const throw() { return msg.c_str(); }
	};

	class badRequest: public exception
	{
		private:	string msg;
		public:		explicit badRequest(): msg("") {}
					explicit badRequest(const string& m): msg(m) {}
					virtual ~badRequest() throw() {};
					virtual const char * what() const throw() { return msg.c_str(); }
	};

	class methodNotAllowed: public exception
	{
		private:	string msg;
		public:		explicit methodNotAllowed(): msg("") {}
					explicit methodNotAllowed(const string& m): msg(m) {}
					virtual ~methodNotAllowed() throw() {};
					virtual const char * what() const throw() { return msg.c_str(); }
	};

private:
	void	dummy() {}

};

char checkMethod(const string& content)
{
	string::size_type end = content.find(" ");
	string method = content.substr(0, end);
	if		(method == "GET")	return GET;
	else if (method == "PUT")	return PUT;
	else if (method == "POST")	return POST;
	else if (method == "DELETE")return DELETE;
	return 0;
}




#endif
