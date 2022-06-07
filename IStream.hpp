#ifndef ISTREAM_HPP
# define ISTREAM_HPP

# include <string>
#include <unistd.h>
using namespace std;

class IStream
{
protected:
	int	fd;

public:
	IStream() {};
	IStream( int _fd ): fd(_fd) {};
	IStream( const IStream& src ): fd(src.fd) {};
	virtual ~IStream() {};

	IStream&	operator=( const IStream& src )
	{
		if (this != &src)
		{
			;

		}
		return *this;
	}

	int				getFD()	const		{ return fd; }
	void			setFD( int fd )		{ this->fd = fd; }
	void			close()				{ ::close(fd); }

	class somethingWrong: public exception
	{
		private:	string msg;
		public:		explicit somethingWrong(const string& m): msg(m) {}
					virtual ~somethingWrong() throw() {};
					virtual const char * what() const throw() { return msg.c_str(); }
	};

private:
	virtual void			dummy() = 0;

};





#endif
