#ifndef SERVERSOCKET_HPP
# define SERVERSOCKET_HPP

# include "ConnSocket.hpp"
# include <exception>
# include <stdexcept>
#include <sys/errno.h>
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
		int any =	::bind(this->fd, (struct sockaddr *)&this->info, sizeof(this->info));
		if (any)	throw somethingWrong(strerror(errno));
		cout << "bind OK" << endl;
	}
	void			listen(int backlog)
	{
		int any =	::listen(this->fd, backlog);
		if (any)	throw somethingWrong(strerror(errno));
		cout << "listen OK" << endl;
	}
	ConnSocket	accept() const
	{
		ConnSocket c;
		c.fd = ::accept(this->fd, (struct sockaddr *)&c.info, &c.len);
		if (c.fd == -1)
		{
			if (errno != EWOULDBLOCK && errno != EAGAIN)	exit(-1);
			else											throw somethingWrong(strerror(errno));
		}

		// struct linger l = {.l_onoff = 1, .l_linger = 15};
		// setsockopt(this->fd, SOL_SOCKET, SO_LINGER, &l, sizeof(l));
		// cout << "accept OK :" << c.fd << endl;
		return c;
	}
private:
	void dummy() {}
};


#endif
