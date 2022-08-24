#include "AStream.hpp"


/**========================================================================
* @                           Constructors
*========================================================================**/

	AStream::AStream()
	: fd(-1), timeout(-1), lastActive(time(NULL)), p(NULL)	{  }
	AStream::AStream( int _fd )
	: fd(_fd), timeout(-1), lastActive(time(NULL)), p(NULL)	{  }
	AStream::AStream( int _fd, time_t _to)
	: fd(_fd), timeout(_to), lastActive(time(NULL)), p(NULL)	{  }
	AStream::AStream( const AStream& src )
	: fd(src.fd), timeout(src.timeout), lastActive(src.lastActive), p(NULL) { }
	AStream::~AStream() {};

/**========================================================================
* *                            operators
*========================================================================**/

	AStream&	AStream::operator=( const AStream& src )
	{
		if (this != &src)
		{
			this->fd = src.fd;
			this->timeout = src.timeout;
			this->lastActive = src.lastActive;
		}
		return *this;
	}


/**========================================================================
* #                          member functions
*========================================================================**/

	int				AStream::getFD()	const		{ return fd; }
	void			AStream::setFD( int fd )		{ this->fd = fd; }
	void			AStream::updateLastActive()		{ this->lastActive = time(NULL); }
	time_t			AStream::getLastActive() const	{ return lastActive; }
	void			AStream::setTimeOut(time_t to)	{ this->timeout = to; }
	time_t			AStream::getTimeOut() const		{ return this->timeout; }
	void			AStream::close()				{ ::close(fd); }

