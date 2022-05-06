/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mishin <mishin@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/06 10:46:37 by mishin            #+#    #+#             */
/*   Updated: 2022/05/06 15:03:01 by mishin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERSOCKET_HPP
# define SERVERSOCKET_HPP

# include "ConnSocket.hpp"
# include <exception>
# include <stdexcept>
# include <sys/socket.h>


class ServerSocket : public ISocket
{
public:
	ServerSocket( const string&			ip,
				  const unsigned short& port )
	: ISocket(ip, port) {};
	~ServerSocket() {};

	void			bind()
	{
		int any =	::bind(this->sock, (struct sockaddr *)&this->info, sizeof(this->info));
		if (any)	throw something_wrong(strerror(errno));
		cout << "bind OK" << endl;
	}
	void			listen(int backlog)
	{
		int any =	::listen(this->sock, backlog);
		if (any)	throw something_wrong(strerror(errno));
		cout << "listen OK" << endl;
	}
	ConnSocket	accept()
	{
		ConnSocket c;
		c.sock = ::accept(this->sock, (struct sockaddr *)&c.info, &c.len);
		if (c.sock == -1)
			exit(-1);
		cout << "accept OK" << endl;
		return c;
	}
};


#endif
