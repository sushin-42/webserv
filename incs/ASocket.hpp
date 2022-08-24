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

#include "AStream.hpp"
# include "color.hpp"

using namespace std;

class ASocket : public AStream
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
	ASocket() : AStream() {};
	ASocket( const string& ip, const unsigned short& port )
	: AStream()
	{
		info.sin_family	= PF_INET;
		setIP(ip);
		setPort(port);
		bzero(info.sin_zero, sizeof(info.sin_zero));

		fd = socket(PF_INET, SOCK_STREAM, 0);
		fcntl(fd, F_SETFL, O_NONBLOCK);
	}
	ASocket( const ASocket& src )
	:AStream(src), info(src.info) {}

	~ASocket() {}

/**========================================================================
* *                            operators
*========================================================================**/

	ASocket&	operator=( const ASocket& src )
	{
		if (this != &src)
		{
			this->::AStream::operator=(src);
			this->info = src.info;
		}
		return *this;
	}

/**========================================================================
* #                          member functions
*========================================================================**/

	string			getIP() const;
	unsigned short	getPort() const;

	//! do not set connected-socket
	void			setIP( const string& ip );
	void			setPort( const unsigned short& port);

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

	void send(const string& s);
private:
	virtual void			dummy() = 0;
};


#endif
