#include "Pipe.hpp"
#include "CGI.hpp"
#include "ConnSocket.hpp"
#include "Exceptions.hpp"

/**========================================================================
* @                           Constructors
*========================================================================**/

	Pipe::Pipe()
	: Stream(-1), output(), pid(0), status(0), headerDone(false), linkConn(NULL) {}

	Pipe::Pipe( int fd, pid_t p )
	:Stream(fd), output(), pid(p), status(0), headerDone(false), linkConn(NULL) {}

	Pipe::Pipe( const Pipe& src )
	: Stream(src), output(src.output), pid(src.pid), status(src.status), headerDone(false), linkConn(src.linkConn) {}

	Pipe::~Pipe() {}

/**========================================================================
* *                            operators
*========================================================================**/

	Pipe&	Pipe::operator=( const Pipe& src )
	{
		if (this != &src)
		{
			this->Stream::operator=(src);
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

void	Pipe::core()
{
	// recv();
}

void	Pipe::recv()
{
	ConnSocket* connected = this->linkConn;

	ssize_t	byte = 0;

	switch (byte = readFrom(this->fd, this->output))
	{

	case -1:	/* internal server error */
		TAG(CGI#, CGIroutines); cout << RED("Unexcpected error from pipe: ") << this->getFD() << endl;
		this->close();
		/* need to return 500! */
		break;

	case 0:		/* close pipe, process output */

		TAG(CGI#, CGIroutines); cout << GRAY("Pipe closed: ") << this->getFD() << endl;
		this->close();

		if (connected->pending == false)
		{
			connected->ResB.setContent(
										connected->chunk ?
											makeChunk(this->output) :
											this->output
									);
		}
		connected->pending = false;
		break;

	default:	/* output appended */

		/* wait full header */
		if (this->headerDone == false)
		{
			connected->pending = true;
			if	(this->output.rfind("\r\n\r\n") != string::npos ||
					this->output.rfind("\n\n") != string::npos)
			{
				processOutputHeader(connected, this);
				this->headerDone = true;							// appended to conn->ResH
				this->output = extractBody(this->output);		// store remained after header
				connected->pending = false;
			}
			else
				return;
		}
		if (connected->pending == false)
		{
			if (this->output.empty())	return; // if extracted trailing Body == '', makeChunk will send '0\r\n\r\n'
			connected->ResB.setContent(
										connected->chunk ?
											makeChunk(this->output) :
											this->output
									);
			this->output.clear();
		}
	}
}

void	Pipe::coreDone()
{
	ConnSocket* connected = this->linkConn;

	// if (headerDone)
	if (connected->pending == false)
		POLLSET->getIterator(connected).first->events |= POLLOUT;
	return;
}

string	Pipe::getOutputContent() { return this->linkConn->ReqB.getContent();  }
void	Pipe::send(const string& content, map<int, struct undone>& writeUndoneBuf)
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
		this->linkConn->linkOutputPipe = NULL;
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
