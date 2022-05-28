#ifndef PIPE_HPP
# define PIPE_HPP
# include "IStream.hpp"
#include <sys/_types/_pid_t.h>

class Pipe : public IStream
{
public:
	string	content;
	pid_t	pid;
	int		status;

public:
	Pipe()
	: IStream(), content(), pid(0), status(0) {};

	Pipe( int fd, pid_t p )
	:IStream(fd), content(), pid(p), status(0) {};

	Pipe( const Pipe& src )
	: IStream(src), content(src.content) {}

	~Pipe() {};

	Pipe&	operator=( const Pipe& src )
	{
		if (this != &src)
		{
			this->IStream::operator=(src);
			this->content = src.content;
		}
		return *this;
	}
private:
	void dummy() {}
};


#endif
