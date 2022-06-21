#ifndef ISTREAM_HPP
# define ISTREAM_HPP

#include <ctime>
#include <map>
# include <string>
#include <unistd.h>
#include <sys/time.h>
#include "Undone.hpp"
using namespace std;

class IStream
{
/**========================================================================
* %                          member variables
*========================================================================**/

protected:
	int		fd;
	time_t	timeout;
	time_t	lastActive;

/**========================================================================
* @                           Constructors
*========================================================================**/

public:
	IStream()
	: fd(-1), timeout(-1), lastActive(time(NULL))	{  }
	IStream( int _fd )
	: fd(_fd), timeout(-1), lastActive(time(NULL))	{  }
	IStream( int _fd, time_t _to)
	: fd(_fd), timeout(_to), lastActive(time(NULL))	{  }
	IStream( const IStream& src )
	: fd(src.fd), timeout(src.timeout), lastActive(src.lastActive) { }
	virtual ~IStream() {};

/**========================================================================
* *                            operators
*========================================================================**/

	IStream&	operator=( const IStream& src )
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

	int				getFD()	const;
	void			setFD( int fd );
	void			updateLastActive();
	time_t			getLastActive() const;
	void			setTimeOut(time_t to);
	time_t			getTimeOut() const;
	void			close();



/**========================================================================
* !                            Exceptions
*========================================================================**/

	class somethingWrong: public exception
	{
		private:	string msg;
		public:		explicit somethingWrong(const string& m): msg(m) {}
					virtual ~somethingWrong() throw() {};
					virtual const char * what() const throw() { return msg.c_str(); }
	};

private:
	virtual void	dummy() = 0;
public:
	virtual void	send(const string& content, map<int, undone>& writeUndoneBuf) = 0;
};





#endif
