/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnSocket.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mishin <mishin@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/06 12:03:16 by mishin            #+#    #+#             */
/*   Updated: 2022/05/11 15:57:39 by mishin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNSOCKET_HPP
# define CONNSOCKET_HPP

# include <iostream>
#include <string>
#include <sys/_types/_ssize_t.h>
#include <sys/errno.h>
# include <utility>

# include <sys/fcntl.h>

# include "ISocket.hpp"
# include "Config.hpp"
# include "color.hpp"

# include "utils.hpp"
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

typedef unsigned short status_code_t;

class ConnSocket : public ISocket
{
friend class ServerSocket;

private:
	socklen_t	len;
	char		recvbuf[1024];
	// char		sendbuf[1024];	//' is required?

public:
	ConnSocket() : ISocket(), len(sizeof(info)) {}
	~ConnSocket() {}

	ConnSocket&	operator=( const ConnSocket& src )
	{
		if (this != &src)
		{
			this->ISocket::operator=(src);
			this->len		= src.len;
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

	ReqHeader recvRequest()
	{
		int method = 0;
		static int count = 0;
		ssize_t	byte = 0;
		string	content;
		bzero(recvbuf, sizeof(recvbuf));
		while ((byte = read(this->sock, this->recvbuf, sizeof(recvbuf))) > 0)
		{
 			content.append(recvbuf, byte);	// '+=' is bad for processing binary data

			if (!method)	method = checkMethod(content);
			switch (method)
			{
			case GET:
				if (content.substr(content.length() - 4) == "\r\n\r\n")
					goto exitloop;		// ' 'break' do not exit the loop due to switch
										// ! read() will return EAGAIN if no to read, need 'break'
				break;
			case PUT:	cerr << "NOT SUPPORT PUT" << endl;		break;
			case POST:	cerr << "NOT SUPPORT POST" << endl;		break;
			case DELETE:cerr << "NOT SUPPORT DELETE" << endl;	break;
			}
			bzero(recvbuf, sizeof(recvbuf));
			// memset(recvbuf, 0, sizeof(recvbuf));
		}
	exitloop:
		ReqHeader req(content);
		if (byte == -1)
		{
			cout << req.getContent() << endl;
			if (errno != EAGAIN && errno != EWOULDBLOCK) exit(100);
			else { cout << RED("RECV REQUEST COUNT ") << count++ << endl; return req;}

		}
		else if (byte == 0)	//TODO: closed by CLIENT
		{
			cout << PURPLE("CLIENT EXIT") << endl;
			if (errno != EAGAIN && errno != EWOULDBLOCK)
				throw something_wrong(strerror(errno));
		}


		return req;
	}

	void	send(const string& content)
	{
		ssize_t	status;
		status = write(this->sock, content.data(), content.length());
		if (status == -1)
			cerr << RED("ConnSocket#write() bad") << endl;
		else
			cout << CYAN("ConnSocket#send() good") << endl;
	}

	void	send(const string& content, int fd)
	{
		ssize_t	status;
		status = write(fd, content.data(), content.length());
		if (status == -1)
			cerr << RED("ConnSocket#write() bad") << endl;
		else
			cout << CYAN("ConnSocket#send() good") << endl;
	}
private:
	void			dummy() {}
};




#endif
