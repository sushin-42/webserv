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

	ssize_t	read()
	{
		ssize_t	byte = readFrom(this->fd, this->output);

		//NOTE: 왜 몰아서 출력되지 ? ㅎ  아! 버퍼링되는구나. print(flush=true)

		return byte;
	}

	void	send(const string& content, map<int, struct undone>& writeUndoneBuf)
	{
		try						{ writeUndoneBuf.at(this->fd); }
		catch (exception& e)	{ writeUndoneBuf[this->fd] = (struct undone){"",0};
								  writeUndoneBuf[this->fd].content.append(content.data(), content.length());	}

		string&		rContent	= writeUndoneBuf[this->fd].content;
		ssize_t&	rWrited		= writeUndoneBuf[this->fd].totalWrited;
		ssize_t		rContentLen	= rContent.length();
		ssize_t		byte		= 0;

		byte = ::write( this->fd,
					  rContent.data() + rWrited,
					//   1);
					  rContentLen - rWrited );
		if (byte > 0)
			rWrited += byte;

		//@ all data sended @//
		if (rWrited == rContentLen)
		{
			TAG(Pipe, send) << _GOOD(all data sended to child process) << this->fd << ": " << rWrited << " / " << rContentLen << " bytes" << endl;
			writeUndoneBuf.erase(this->fd);
			close();
			throw exception();
		}
		//' not all data sended. have to be buffered '//
		else
		{
			TAG(Pipe, send) << _NOTE(Not all data sended to) << this->fd << ": " << rWrited << " / " << rContentLen  << " bytes" << endl;
			if (byte == -1)
			{
				TAG(Pipe, send) << _FAIL(unexpected error: ) << errno << endl;
				writeUndoneBuf.erase(this->fd);
				close();
				throw exception();	// close and Drop now!
									// NOTE: need to drop linkConn?
			}
			throw sendMore();
		}
	}

private:
	void dummy() {}
};

#endif
