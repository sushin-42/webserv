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
	ConnSocket	accept() const
	{
		ConnSocket c;
		c.sock = ::accept(this->sock, (struct sockaddr *)&c.info, &c.len);
		if (c.sock == -1)
		{
			if (errno != EWOULDBLOCK && errno != EAGAIN)	exit(-1);
			else											throw something_wrong(strerror(errno));
		}
		// cout << "accept OK :" << c.sock << endl;
		return c;
	}
private:
	void dummy() {}
};


#endif
