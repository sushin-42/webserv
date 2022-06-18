#ifndef PIPE_HPP
# define PIPE_HPP
# include "utils.hpp"
# include "IStream.hpp"
# include "Undone.hpp"
# include <sys/_types/_pid_t.h>

class ConnSocket;

class Pipe : public IStream
{
/**========================================================================
* %                          member variables
*========================================================================**/

public:
	string	output;
	pid_t	pid;
	int		status;
	bool	headerDone;
	ConnSocket*	linkConn;


/**========================================================================
* @                           Constructors
*========================================================================**/

public:
	Pipe()
	: IStream(-1), output(), pid(0), status(0), headerDone(false), linkConn(NULL) {};

	Pipe( int fd, pid_t p )
	:IStream(fd), output(), pid(p), status(0), headerDone(false), linkConn(NULL) {};

	Pipe( const Pipe& src )
	: IStream(src), output(src.output), pid(src.pid), status(src.status), headerDone(false), linkConn(src.linkConn) {}

	~Pipe() {};

/**========================================================================
* *                            operators
*========================================================================**/

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

/**========================================================================
* #                          member functions
*========================================================================**/

	ssize_t	read();
	void	send(const string& content, map<int, struct undone>& writeUndoneBuf);
private:
	void dummy() {}
};

#endif