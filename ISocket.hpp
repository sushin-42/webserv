/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ISocket.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mishin <mishin@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/06 10:32:52 by mishin            #+#    #+#             */
/*   Updated: 2022/05/11 15:33:44 by mishin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ISOCKET_HPP
# define ISOCKET_HPP

# include <iostream>
# include <netinet/in.h>
# include <cstring>
# include <sys/fcntl.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <string>
# include <unistd.h>

# include "color.hpp"

using namespace std;
// struct sockaddr_in;
// struct sockaddr;

class ISocket
{
protected:
	typedef struct sockaddr_in socket_info_t;
	int				sock;
	socket_info_t	info;

public:
	ISocket() {};
	ISocket( const string& ip, const unsigned short& port )
	{
		info.sin_family	= PF_INET;
		setIP(ip);
		setPort(port);
		bzero(info.sin_zero, sizeof(info.sin_zero));

		sock = socket(PF_INET, SOCK_STREAM, 0);
		fcntl(sock, F_SETFL, O_NONBLOCK | SO_REUSEADDR);
	}
	ISocket( const ISocket& src )
	:sock(src.sock), info(src.info) {}

	virtual ~ISocket() {}

	ISocket&	operator=( const ISocket& src )
	{
		if (this != &src)
		{
			this->sock = src.sock;
			this->info = src.info;
		}
		return *this;
	}

	string			getIP() const		{ return inet_ntoa(info.sin_addr); }
	unsigned short	getPort() const		{ return ntohs(info.sin_port); }
	int				getFD()	const		{ return sock; }
	void			close()				{ ::close(sock); cout << RED("close ") << sock << endl; }

	//! do not set connected-socket
	void			setIP( const string& ip )				{ if (ip ==  "")	this->info.sin_addr.s_addr = INADDR_ANY;
															  else				this->info.sin_addr.s_addr = inet_addr(ip.c_str()); }
	void			setPort( const unsigned short& port)	{ this->info.sin_port = htons(port); }
	void			setFD( int fd )							{ this->sock = fd; }

	class something_wrong: public exception
	{
		private:	string msg;
		public:		explicit something_wrong(const string& m): msg(m) {}
					virtual ~something_wrong() throw() {};
					virtual const char * what() const throw() { return msg.c_str(); }
	};


private:
	virtual void			dummy() = 0;
};


#endif
