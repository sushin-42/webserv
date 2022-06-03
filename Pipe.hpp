#ifndef PIPE_HPP
# define PIPE_HPP
# include "ConnSocket.hpp"
# include "IStream.hpp"
#include <sys/_types/_pid_t.h>

class Pipe : public IStream
{
public:
	string	output;
	pid_t	pid;
	int		status;
	bool	headerDone;
	ConnSocket*	linkConn;

public:
	Pipe()
	: IStream(-1), output(), pid(0), status(0), headerDone(false), linkConn(NULL) {};

	Pipe( int fd, pid_t p )
	:IStream(fd), output(), pid(p), status(0), headerDone(false), linkConn(NULL) {};

	Pipe( const Pipe& src )
	: IStream(src), output(src.output), pid(src.pid), status(src.status), headerDone(false), linkConn(src.linkConn) {}

	~Pipe() {};

	Pipe&	operator=( const Pipe& src )
	{
		if (this != &src)
		{
			this->IStream::operator=(src);
			this->output 	= src.output;
			this->pid	 	= src.pid;
			this->status 	= src.status;
			this->headerDone= src.headerDone;
			this->linkConn	= src.linkConn;
		}
		return *this;
	}

	ssize_t	read()
	{
		ssize_t	byte = readFrom(this->fd, this->output);

		//NOTE: 왜 몰아서 출력되지 ? ㅎ  아! 버퍼링되는구나. print(flush=true)

		return byte;
	}

	bool	isProcessing()	{ return (fd != -1); }

private:
	void dummy() {}
};

#endif
