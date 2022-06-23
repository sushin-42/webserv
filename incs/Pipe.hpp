#ifndef PIPE_HPP
# define PIPE_HPP
# include "utils.hpp"
# include "Stream.hpp"
# include "Undone.hpp"
# include <sys/_types/_pid_t.h>

class ConnSocket;

class Pipe : public Stream
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
	Pipe();
	Pipe( int fd, pid_t p );
	Pipe( const Pipe& src );
	~Pipe();

/**========================================================================
* *                            operators
*========================================================================**/

	Pipe&	operator=( const Pipe& src );

/**========================================================================
* #                          member functions
*========================================================================**/

	void	recv();
	void	coreDone();
	void	send(const string& content, map<int, struct undone>& writeUndoneBuf);
	void	core();
	string	getOutputContent();
private:
	void dummy() {}
};

#endif
