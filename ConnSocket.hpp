#ifndef CONNSOCKET_HPP
# define CONNSOCKET_HPP

#include <cstring>
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

struct undone
{
	string	content;
	ssize_t	totalWrited;
};

class ConnSocket : public ISocket
{
friend class ServerSocket;

private:
	socklen_t	len;
	char		recvbuf[1024];
	// char		sendbuf[1024];	//' is required?

public:
	ReqHeader	ReqH;
	ReqBody		ReqB;
	ResHeader	ResH;
	ResBody		ResB;
	bool		pending;
	bool		chunk;	/* to distinguish script output chunk with server chunk */
	Pipe*		linkPipe;

public:
	ConnSocket() : ISocket(), len(sizeof(info)), ReqH(), ReqB(), ResH(), ResB(), pending(false), chunk(false), linkPipe(NULL)  {}
	~ConnSocket() {}

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
			this->linkPipe	= src.linkPipe;
			//NOTE: no buf copy
		}
		return *this;
	}

	char checkMethod(const string& src)
	{
		string::size_type end = src.find(" ");
		string method = src.substr(0, end);
		if		(method == "GET")	return GET;
		else if (method == "PUT")	return PUT;
		else if (method == "POST")	return POST;
		else if (method == "DELETE")return DELETE;
		return -1;
	}

	void	recvRequest()	//TODO: seperate and return both Header and Body
	{
		// ReqHeader	ReqH;
		// ReqBody		ReqB;
		int			method	= 0;
		string		content = "";
		ssize_t		byte	= 0;
		bzero(recvbuf, sizeof(recvbuf));
		while ((byte = read(this->fd, this->recvbuf, sizeof(recvbuf))) > 0)
		{
 			content.append(recvbuf, byte);	// '+=' is bad for processing binary data

			if (!method)	method = checkMethod(content);
			switch (method)
			{
			case GET:	ReqH.setMethod("GET");		break;
			case PUT:	ReqH.setMethod("PUT");		break;
			case POST:	ReqH.setMethod("POST");		break;
			case DELETE:ReqH.setMethod("DELETE");	break;
			}
			bzero(recvbuf, sizeof(recvbuf));
		}
		ReqH.setHTTPversion("HTTP/1.1");	//TODO: parse from request
		ReqH.setRequsetTarget(content);
		ReqH.setContent(extractHeader(content));
		ReqB.setContent(extractBody(content));
		ReqH.setHeaderField(KVtoMap(content, ':'));
		if (ReqH.headerField["transfer-encoding"] == "chunked")
		{
			try
			{
				ReqB.decodingChunk();
				// 아래는 확인 차 출력
				cout << ReqB.getContent() << endl;
			}
			catch(const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}
		}
		else if (ReqH.headerField.find("content-length") != ReqH.headerField.end())
		{
			try
			{
				ReqB.checkContentLength(ReqH.headerField["content-length"]);
			}
			catch(const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}
		}
		if (byte == -1)
		{
			if (errno != EAGAIN && errno != EWOULDBLOCK)
				throw something_wrong(strerror(errno));
			else
			{
				TAG(ConnSocket, recvRequest) << YELLOW("No data to read") << endl;
				// return make_pair(ReqH, ReqB);
			}
		}
		else if (byte == 0)	// closed by CLIENT
		{
			TAG(ConnSocket, recvRequest); cout << GRAY("CLIENT EXIT ") << this->fd << endl;
			// return make_pair(ReqH, ReqB);
		}
		// return make_pair(ReqH, ReqB);
	}

	void	send(const string& content, map<int, undone>& buf)
	{
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
			TAG(ConnSocket, send) << _GOOD(server send FIN: ) << _UL << this->fd << _NC << endl;
			shutdown(this->fd, SHUT_WR);
		}
		else
			TAG(ConnSocket, send) << GRAY("WHY YOU HERE?") << endl;

	}

private:
	void			dummy() {}
};




#endif
