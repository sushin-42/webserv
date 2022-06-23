#include "ServerSocket.hpp"
#include "ConfigLoader.hpp"
#include "Undone.hpp"

/**========================================================================
* #                          member functions
*========================================================================**/

	void			ServerSocket::bind()
	{
		int any =	::bind(this->fd, (struct sockaddr *)&this->info, sizeof(this->info));
		if (any)	throw somethingWrong(strerror(errno));
		cout << "bind OK" << endl;
	}
	void			ServerSocket::listen(int backlog)
	{
		int any =	::listen(this->fd, backlog);
		if (any)	throw somethingWrong(strerror(errno));
		cout << "listen OK" << endl;
	}
	ConnSocket		ServerSocket::accept() const
	{
		ConnSocket c;

		c.fd			 	= ::accept(this->fd, (struct sockaddr *)&c.info, &c.len);
		c.conf				= ConfigLoader::_()->getDefaultServer(this->getIP(), this->getPort());
		c.linkServerSock	= const_cast<ServerSocket*>(this);
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
	void		ServerSocket::send(const string& s, map<int, struct undone>& u) { (void)s, (void)u; }
	void		ServerSocket::core() {}
	void		ServerSocket::recv() {}
	void		ServerSocket::dummy() {}

