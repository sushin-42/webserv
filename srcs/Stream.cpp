#include "Stream.hpp"


/**========================================================================
* @                           Constructors
*========================================================================**/

	Stream::Stream()
	: fd(-1), timeout(-1), lastActive(time(NULL)), p(NULL)	{  }
	Stream::Stream( int _fd )
	: fd(_fd), timeout(-1), lastActive(time(NULL)), p(NULL)	{  }
	Stream::Stream( int _fd, time_t _to)
	: fd(_fd), timeout(_to), lastActive(time(NULL)), p(NULL)	{  }
	Stream::Stream( const Stream& src )
	: fd(src.fd), timeout(src.timeout), lastActive(src.lastActive), p(NULL) { }
	Stream::~Stream() {};

/**========================================================================
* *                            operators
*========================================================================**/

	Stream&	Stream::operator=( const Stream& src )
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

	int				Stream::getFD()	const		{ return fd; }
	void			Stream::setFD( int fd )		{ this->fd = fd; }
	void			Stream::updateLastActive()		{ this->lastActive = time(NULL); }
	time_t			Stream::getLastActive() const	{ return lastActive; }
	void			Stream::setTimeOut(time_t to)	{ this->timeout = to; }
	time_t			Stream::getTimeOut() const		{ return this->timeout; }
	void			Stream::close()				{ ::close(fd); }

