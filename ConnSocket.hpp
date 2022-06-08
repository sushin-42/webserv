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
#include "utils.hpp"

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
struct undone
{
	string	content;
	ssize_t	totalWrited;
};

class ConnSocket : public ISocket
{
friend class ServerSocket;

/**========================================================================
* %                          member variables
*========================================================================**/

private:
	socklen_t	len;
	char		buf[1024];
	string		recvContent;	// cumulate received content

public:
	ReqHeader	ReqH;
	ReqBody		ReqB;
	ResHeader	ResH;
	ResBody		ResB;
	bool		pending;
	bool		chunk;		/* to distinguish script output chunk with server chunk */
	bool		FINsended;	/* we already sended FIN, DO NOT send more data. */

	Pipe*		linkPipe;

/**========================================================================
* @                           Constructors
*========================================================================**/

public:
	ConnSocket() : ISocket(), len(sizeof(info)), recvContent(), ReqH(), ReqB(), ResH(), ResB(), pending(false), chunk(false), FINsended(false), linkPipe(NULL)  {}
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
			this->linkPipe	= src.linkPipe;
		}
		return *this;
	}

/**========================================================================
* #                          member functions
*========================================================================**/

	void	setHeaderOrReadMore()
	{
		if (has2CRLF(recvContent))	//NOTE: what if bad-format request doesn't contain "\r\n\r\n"?
		{
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
				what if trailing header, or something exists after "0\r\n\r\n" ?
				what if payload contains "0\r\n\r\n" ?
			*/
			if (recvContent.substr(recvContent.length()-5) == "0\r\n\r\n")
			{
				ReqB.setContent(recvContent);
				ReqB.decodingChunk();
			}
			else
				throw readMore();
		}
		else if (ReqH.exist("Content-Length"))
		{
			if (!isNumber(ReqH["Content-Length"]))
				throw badRequest();
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
		while ((byte = read(this->fd, this->buf, sizeof(buf))) > 0)
		{
 			recvContent.append(buf, byte);
			bzero(buf, sizeof(buf));
		}

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

	void	send(const string& content, map<int, undone>& buf)
	{
		if (FINsended) return;
		try						{ buf.at(this->fd); }
		catch (exception& e)	{ buf[this->fd] = (struct undone){"",0};
								  buf[this->fd].content.append(content.data(), content.length());	}

		string&		rContent	= buf[this->fd].content;
		ssize_t&	rWrited		= buf[this->fd].totalWrited;
		ssize_t		rContentLen	= rContent.length();
		ssize_t		byte		= 0;
		pid_t		pid			= 0;

		while ( true )
		{
			byte = write( this->fd,
						  rContent.data() + rWrited,
						  rContentLen - rWrited );

			if (byte <= 0)				break;

			rWrited += byte;
			if (rWrited == rContentLen)	break; //@ send GOOD, stop write()

		}

		if (byte == -1)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
			{
				if (rWrited == rContentLen)	//! means all data sended. cannot reach here?
					TAG(ConnSocket, send) << _NOTE(No data to write) << endl;
				else						//' not all data sended. have to be buffered.
					TAG(ConnSocket, send) << _NOTE(Not all data sended to) << this->fd << ": " << rWrited << " / " << rContentLen  << " bytes" << endl;
				throw exception();
			}
			else
				TAG(ConnSocket, send) << _FAIL(unexpected error) << errno << endl;
		}
		else if (rWrited == rContentLen)
		{
			TAG(ConnSocket, send) << _GOOD(all data sended to) << this->fd << ": " << rWrited << " / " << rContentLen << " bytes" << endl;
			buf.erase(this->fd);
			if (linkPipe)
			{
				pid = waitpid(linkPipe->pid, &linkPipe->status, WNOHANG);
				if (!(pid == linkPipe->pid || pid == -1))
					return ;
				else
					TAG(ConnSocket, send) << _GOOD(waitpid on ) << linkPipe->pid << CYAN(" returns ") << _UL << pid << _NC << endl;

			}
			//NOTE: what if client doesn't send FIN? now we send FIN and close "after client send FIN too".
			// Have to close() instantly after send FIN, with short timer.
			shutdown(this->fd, SHUT_WR);
			FINsended = true;	/*
									for case that client keep sending message even after FIN.
									we prevent calling ConnSocket#send().
								*/
			TAG(ConnSocket, send) << _GOOD(server send FIN: ) << _UL << this->fd << _NC << endl;
		}
		else
			TAG(ConnSocket, send) << GRAY("WHY YOU HERE?") << endl;

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

	class readMore: public exception
	{
		private:	string msg;
		public:		explicit readMore(): msg("") {}
					explicit readMore(const string& m): msg(m) {}
					virtual ~readMore() throw() {};
					virtual const char * what() const throw() { return msg.c_str(); }
	};

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
