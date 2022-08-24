#include "ASocket.hpp"

/**========================================================================
* #                          member functions
*========================================================================**/

	string			ASocket::getIP() const		{ return inet_ntoa(info.sin_addr); }
	unsigned short	ASocket::getPort() const	{ return ntohs(info.sin_port); }

	//! do not set connected-socket
	void			ASocket::setIP( const string& ip )				{ if (ip ==  "")	this->info.sin_addr.s_addr = INADDR_ANY;
															  		  else				this->info.sin_addr.s_addr = inet_addr(ip.c_str()); }
	void			ASocket::setPort( const unsigned short& port)	{ this->info.sin_port = htons(port); }
	void			ASocket::send(const string& s)					{(void)s; }
