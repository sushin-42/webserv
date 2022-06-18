#include "IStream.hpp"

/**========================================================================
* #                          member functions
*========================================================================**/

	int				IStream::getFD()	const		{ return fd; }
	void			IStream::setFD( int fd )		{ this->fd = fd; }
	void			IStream::updateLastActive()		{ this->lastActive = time(NULL); }
	time_t			IStream::getLastActive() const	{ return lastActive; }
	void			IStream::setTimeOut(time_t to)	{ this->timeout = to; }
	time_t			IStream::getTimeOut() const		{ return this->timeout; }
	void			IStream::close()				{ ::close(fd); }
