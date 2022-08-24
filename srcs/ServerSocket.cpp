#include "ServerSocket.hpp"
#include "ConfigLoader.hpp"
#include "Exceptions.hpp"



/**========================================================================
* #                          member functions
*========================================================================**/

	void			ServerSocket::bind()
	{
		int any =	::bind(this->fd, (struct sockaddr *)&this->info, sizeof(this->info));
		cout << _GREEN << "bind " << _UL << getIP() + ":" + toString(getPort()) << _NC << "\t-> "; 
		if (any)	{  cout << RED("FAIL: "); throw somethingWrong(strerror(errno)); }
		cout << CYAN("OK") << endl;
	}
	void			ServerSocket::listen(int backlog)
	{
		int any =	::listen(this->fd, backlog);
		cout << _GREEN << "listen " << _UL << getIP() + ":" + toString(getPort()) << _NC << "\t-> "; 
		if (any)	{  cout << RED("FAIL: "); throw somethingWrong(strerror(errno)); }
		cout << CYAN("OK") << endl;

	}
	ConnSocket		ServerSocket::accept() const
	{
		ConnSocket c;

		c.fd			 	= ::accept(this->fd, (struct sockaddr *)&c.info, &c.len);
		c.conf				= ConfigLoader::_()->getDefaultServer(this->getIP(), this->getPort());
		c.linkServerSock	= const_cast<ServerSocket*>(this);
		if (c.fd == -1)
		{
			if (errno != EWOULDBLOCK && errno != EAGAIN)	throw somethingWrong(strerror(errno));
			else											throw readMore();
		}

		return c;
	}
	void		ServerSocket::send(const string& s) { (void)s; }
	void		ServerSocket::core() {}
	void		ServerSocket::recv() {}
	void		ServerSocket::dummy() {}
	void		ServerSocket::coreDone() {}
	string		ServerSocket::getOutputContent() { return ""; }

