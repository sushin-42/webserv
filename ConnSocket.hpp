/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnSocket.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mishin <mishin@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/06 12:03:16 by mishin            #+#    #+#             */
/*   Updated: 2022/05/06 21:12:27 by mishin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNSOCKET_HPP
# define CONNSOCKET_HPP

# include <iostream>
#include <string>
# include <utility>

# include <sys/fcntl.h>

# include "ISocket.hpp"
# include "Config.hpp"
# include "HeaderTemplate.hpp"
# include "utils.hpp"

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

typedef unsigned short status_code_t;

class ConnSocket : public ISocket
{
friend class ServerSocket;

private:
	socklen_t	len;
	string		reqHeader;
	char		recvbuf[1024];
	char		sendbuf[1024];

public:
	ConnSocket() : ISocket(), len(sizeof(info)), reqHeader() {};
	~ConnSocket() {};

	ConnSocket&	operator=( const ConnSocket& src )
	{
		if (this != &src)
		{
			this->ISocket::operator=(src);
			this->len		= src.len;
			this->reqHeader = src.reqHeader;
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

	string getRequest()
	{
		ssize_t	byte;
		int		method = 0;
		bzero(recvbuf, sizeof(recvbuf));
		while ( (byte = read(this->sock, this->recvbuf, sizeof(recvbuf))) > 0)
		{
			reqHeader += recvbuf;	//NOTE: if read binary from req?
			if (!method)
				switch (method = checkMethod(reqHeader))
				{
				case GET:
					if (reqHeader.substr(reqHeader.length() - 4) == "\r\n\r\n")
						goto exitloop;		// ' 'break' do not exit the loop due to switch
											// ! read() will be blocked until EOF, need 'break'
					break;
				case PUT:	cerr << "NOT SUPPORT PUT" << endl;		break;
				case POST:	cerr << "NOT SUPPORT POST" << endl;		break;
				case DELETE:cerr << "NOT SUPPORT DELETE" << endl;	break;
				}
			bzero(recvbuf, sizeof(recvbuf));
		}
	exitloop:
		if (byte == -1)	throw something_wrong(strerror(errno));

		return reqHeader;
	}

	string	getPath()
	{
		string::size_type start = reqHeader.find(" ") + 1;
		string::size_type end	= reqHeader.find(" ", start);

		return reqHeader.substr(start, end - (start));
	}

	pair<int, status_code_t>	checkFile()	// return (requested fd, status code)
	{
		string	path		= root + getPath();
		int		requested	= open(path.c_str(), O_RDONLY);

		// if (requested == -1)	throw something_wrong(strerror(errno));
		if (requested == -1)	return make_pair(requested, 404);

		return make_pair(requested, 200);
	}

	void	replaceToken(string& header, const string& token, const string& value)
	{
		header.replace(header.find(token), token.length(), value);
	}

	string	makeResponseHeader(status_code_t status)
	{
		string	header			= headerTemplate;
		string	reason;

		replaceToken(header, "#HTTP-VERSION", "HTTP/1.1");
		replaceToken(header, "#STATUS", toString(status));
		switch (status)
		{
		case 200:	reason = "OK"; break;
		case 404:	reason = "NOT FOUND"; break;
		}
		replaceToken(header, "#REASON-PHARSE", reason);
		replaceToken(header, "#MIME-TYPE", MIME[getExt(getPath())]);
		replaceToken(header, "#CONNECTION", "keep-alive");

		return header;
	}

	void	sendResponseHeader(const string& header)
	{
		write(this->sock, header.c_str(), header.length());
		write(STDOUT_FILENO, header.c_str(), header.length());
	}

	void	sendResponseBody(int fd)
	{
		ssize_t	byte = 0;
		bzero(sendbuf, sizeof(sendbuf));
		while ( (byte = read(fd, sendbuf, sizeof(sendbuf))) > 0 )
		{
			write(this->sock, sendbuf, byte);
			bzero(sendbuf, sizeof(sendbuf));
		}
	}

	void	sendResponse()
	{
		pair<int, status_code_t>	fd_status	= checkFile();

		sendResponseHeader( makeResponseHeader(fd_status.second) );
		sendResponseBody(fd_status.first);
	}
};




#endif
