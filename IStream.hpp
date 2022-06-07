#ifndef ISTREAM_HPP
# define ISTREAM_HPP

# include <string>
#include <unistd.h>
#include <sys/time.h>
using namespace std;

class IStream
{
protected:
	int	fd;
	struct timeval time;

public:
	IStream() { gettimeofday(&time, NULL); };
	IStream( int _fd ): fd(_fd) { gettimeofday(&time, NULL); };
	IStream( const IStream& src ): fd(src.fd) { this->time = src.time; };
	virtual ~IStream() {};

	IStream&	operator=( const IStream& src )
	{
		if (this != &src)
		{
			this->time = src.time;
		}
		return *this;
	}

	int				getFD()	const		{ return fd; }
	void			setFD( int fd )		{ this->fd = fd; }
	struct timeval	getTime() const		{ return time; }
	void			setTime()			{ gettimeofday(&time, NULL); }
	void			close()				{ ::close(fd); }

	class something_wrong: public exception
	{
		private:	string msg;
		public:		explicit something_wrong(const string& m): msg(m) {}
					virtual ~something_wrong() throw() {};
					virtual const char * what() const throw() { return msg.c_str(); }
	};

private:
	virtual void			dummy() = 0;

};





#endif
