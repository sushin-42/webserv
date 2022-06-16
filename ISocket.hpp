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

#include "IStream.hpp"
# include "color.hpp"

using namespace std;

class ISocket : public IStream
{
/**========================================================================
* '                              typedefs
*========================================================================**/

protected:
	typedef struct sockaddr_in socket_info_t;

/**========================================================================
* %                          member variables
*========================================================================**/

	socket_info_t	info;

/**========================================================================
* @                           Constructors
*========================================================================**/

public:
	ISocket() : IStream() {};
	ISocket( const string& ip, const unsigned short& port )
	: IStream()
	{
		info.sin_family	= PF_INET;
		setIP(ip);
		setPort(port);
		bzero(info.sin_zero, sizeof(info.sin_zero));

		fd = socket(PF_INET, SOCK_STREAM, 0);
		fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK | SO_REUSEADDR);
	}
	ISocket( const ISocket& src )
	:IStream(src), info(src.info) {}

	~ISocket() {}

/**========================================================================
* *                            operators
*========================================================================**/

	ISocket&	operator=( const ISocket& src )
	{
		if (this != &src)
		{
			this->::IStream::operator=(src);
			this->info = src.info;
		}
		return *this;
	}

/**========================================================================
* #                          member functions
*========================================================================**/

	string			getIP() const		{ return inet_ntoa(info.sin_addr); }
	unsigned short	getPort() const		{ return ntohs(info.sin_port); }

	//! do not set connected-socket
	void			setIP( const string& ip )				{ if (ip ==  "")	this->info.sin_addr.s_addr = INADDR_ANY;
															  else				this->info.sin_addr.s_addr = inet_addr(ip.c_str()); }
	void			setPort( const unsigned short& port)	{ this->info.sin_port = htons(port); }

/**========================================================================
* !                            Exceptions
*========================================================================**/

	class somethingWrong: public exception
	{
		private:	string msg;
		public:		explicit somethingWrong(): msg() {}
					explicit somethingWrong(const string& m): msg(m) {}
					virtual ~somethingWrong() throw() {};
					virtual const char * what() const throw() { return msg.c_str(); }
	};


	void send(const string& s, map<int, struct undone>& u) {(void)s; (void)u;};
private:
	virtual void			dummy() = 0;
};


#endif
