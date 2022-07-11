#include "ISocket.hpp"

/**========================================================================
* #                          member functions
*========================================================================**/

	string			ISocket::getIP() const		{ return inet_ntoa(info.sin_addr); }
	unsigned short	ISocket::getPort() const	{ return ntohs(info.sin_port); }

	//! do not set connected-socket
	void			ISocket::setIP( const string& ip )				{ if (ip ==  "")	this->info.sin_addr.s_addr = INADDR_ANY;
															  		  else				this->info.sin_addr.s_addr = inet_addr(ip.c_str()); }
	void			ISocket::setPort( const unsigned short& port)	{ this->info.sin_port = htons(port); }
	void			ISocket::send(const string& s)					{(void)s; }
