#include "Poll.hpp"
#include "FileStream.hpp"
#include "Pipe.hpp"
#include "Stream.hpp"
#include "Timer.hpp"
#include "WriteUndoneBuf.hpp"
#include "Exceptions.hpp"
#include <ios>

PollSet*	PollSet::pollset;

/**========================================================================
* @                           Constructors
*========================================================================**/

PollSet::PollSet(): pollVec(), pollMap(), timer(NULL) { }
PollSet::~PollSet() {}


/**========================================================================
* #                          member functions
*========================================================================**/


void		PollSet::enroll( Stream* stream, short events )
{
	Poll		p;
	p.fd		= stream->getFD();
	p.events	= events;
	p.revents	= 0;

	stream->setTimeOut(20);
	stream->updateLastActive();

	pollMap[p.fd] = make_pair(p, stream);

#	ifdef DEBUG
	ServerSocket*	serv		= CONVERT(stream, ServerSocket);
	ConnSocket*		connected	= CONVERT(stream, ConnSocket);
	FileStream*		filestream	= CONVERT(stream, FileStream);
	Pipe*			P			= CONVERT(stream, Pipe);

	LOGGING(PollSet, GRAY("Enroll ") "%d" " %s" ,stream->getFD(),
		(serv		? CYAN( " (ServerSocket) ") :
		connected	? BLUE( " (ConnSocket)") :
		filestream	? YELLOW( " (FileStream) ") :
		P			? PURPLE( " (Pipe)") : ""));
#endif
}






void	PollSet::_drop( int fd )
{
	Poll	p;
	Stream* s;

	p = this->pollMap[fd].first;
	s = this->pollMap[fd].second;
	ConnSocket* connSock = CONVERT(s, ConnSocket);
	if (connSock)
	{
		LOGGING(PollSet, GRAY("Drop ") "%d " BLUE("(ConnSocket)"),  fd);
		connSock->unlinkAll();
	}

	else if (CONVERT(s, Pipe))
	{
		LOGGING(PollSet, GRAY("Drop ") "%d " PURPLE("(Pipe)"),  fd);
	}

	else if (CONVERT(s, FileStream))
	{
		LOGGING(PollSet, GRAY("Drop ") "%d " YELLOW("(FileStream)"),  fd);
	}

	delete s;
	pollMap.erase(fd);
	UNDONEBUF->erase(fd);
	close(fd);

}

void	PollSet::drop( Stream* stream ) { _drop(stream->getFD()); }
void	PollSet::drop( int fd )			{ _drop(fd); }

void	PollSet::makePollVec()
{
	Pipe*		p = NULL;
	FileStream* f = NULL;
	_Map::iterator it, ite;
	it = pollMap.begin(), ite = pollMap.end();

	pollVec.clear();

	for ( ; it != ite;)
	{
		p = CONVERT(it->second.second, Pipe);
		f = CONVERT(it->second.second, FileStream);
		if ((p && p->linkConn == NULL) || (f && f->linkConn == NULL))
		{
			drop((it++)->second.second);
		}
		else
		{
			pollVec.push_back(it->second.first);
			++it;
		}
	}

}

vector<Stream*>	PollSet::examine()
{
	int		numReady = 0;
	time_t	minRemaining = 2000;;

	if (timer)
	{
		if ((minRemaining = getMinimumRemaining() * 1000) < 0)
			minRemaining = 2000;
		dropTimeout();
	}

	makePollVec();
	_Vp::iterator it, ite;
	vector<Stream*> ret;
	it = pollVec.begin(), ite = pollVec.end();

	switch (numReady = ::poll(pollVec.data(), pollVec.size(), minRemaining/*time-out*/))
	{
	case -1: LOGGING(PollSet, RED("poll() ERROR: ") "%s", strerror(errno));
			 break;
	case  0: LOGGING(PollSet, GRAY("No event within ") "%lu ms",  minRemaining);
			 break;
	default:;
	}

	for (; it < ite; it++)
	{
		if		(it->revents == 0)			continue;
		else
		{
			pollMap[it->fd].first.revents = it->revents;
			Stream* stream = pollMap[it->fd].second;
			//IMPL: separate updateLastWrite, updateLastRead for send_timeout
			pollMap[it->fd].second->updateLastActive();
			if 			(it->revents & POLLOUT)		ret.push_back( writeRoutine(stream) );
			else if		(it->revents & POLLIN)		ret.push_back( readRoutine(stream) );
		}
	}
	if (ret.empty())
		throw exception();
	return ret;
}

Stream*				PollSet::readRoutine(Stream* stream)
{
	ServerSocket*	serv		= CONVERT(stream, ServerSocket);
	ConnSocket*		connected	= CONVERT(stream, ConnSocket);
	Pipe*			CGIpipe		= CONVERT(stream, Pipe);
	FileStream*		filestream	= CONVERT(stream, FileStream);

	if (serv)
	{
		Poll p;
		p.fd = serv->getFD();
		while(1)	// accept() will throw exception if not readied
		{
			try
			{
				connected = new ConnSocket(serv->accept());
				LOGGING(PollSet, GREEN("Server Got new connection, enroll ") "%d" , connected->getFD());
				this->enroll(connected, POLLIN);
			}
			catch (ISocket::somethingWrong& e)	{ return(serv); }		// accept() unexpected fail
			catch (readMore& e)					{ continue;	}	// accept() not ready
		
			return (serv);
		}
	}
	else if (connected)
	{
		LOGGING(PollSet, GREEN("New data to read ") "%d %s"	, stream->getFD(), BLUE(" (ConnSocket)"));
		return connected;
	}
	else if (CGIpipe)
	{
		LOGGING(PollSet, GREEN("New data to read ") "%d %s"	, stream->getFD(), PURPLE(" (Pipe)"));
		return CGIpipe;
	}
	else if (filestream)
	{
		LOGGING(PollSet, GREEN("New data to read ") "%d %s"	, stream->getFD(), YELLOW(" (FileStream)"));
		return filestream;
	}
	else
	{
		LOGGING(PollSet, RED("Unknown type "));
		throw exception();
	}
}

Stream*	PollSet::writeRoutine(Stream* stream)
{
	LOGGING(PollSet, GREEN("Can write to ") "%d", stream->getFD());
	return stream;
}







const Poll&		PollSet::getPoll(const Stream* const stream) const				{ return _getPoll(stream->getFD()); }
const Poll&		PollSet::getPoll(int fd) const									{ return _getPoll(fd); }
const Poll&		PollSet::_getPoll(int fd) const									{ return (getValueIfExists(pollMap, fd).first); }

void			PollSet::setEvent(const Stream* const stream, short event)		{ _setEvent(stream->getFD(), event); }
void			PollSet::setEvent(int fd, short event)							{ _setEvent(fd, event); }
void			PollSet::_setEvent(int fd, short event)							{ pollMap[fd].first.events |= event; }

void			PollSet::unsetEvent(const Stream* const stream, short event)	{ _unsetEvent(stream->getFD(), event); }
void			PollSet::unsetEvent(int fd, short event)						{ _unsetEvent(fd, event); }
void			PollSet::_unsetEvent(int fd, short event)						{ pollMap[fd].first.events &= ~event; }

void			PollSet::prepareSend( int fd )									{ _setEvent(fd, POLLOUT); }
void			PollSet::prepareSend( const Stream* const stream )				{ _setEvent(stream->getFD(), POLLOUT); }

void			PollSet::unsetSend( int fd )									{ _unsetEvent(fd, POLLOUT); }
void			PollSet::unsetSend( const Stream* const stream )				{ _unsetEvent(stream->getFD(), POLLOUT); }

short			PollSet::getCatchedEvent(const Stream* const stream) const		{ return _getPoll(stream->getFD()).revents; }
short			PollSet::getCatchedEvent(int fd) const							{ return _getPoll(fd).revents; }


void	PollSet::createMonitor() { this->timer = new Timer(); }
time_t	PollSet::getMinimumRemaining()
{
	timer->monitor();
	if (timer->min.first == -1)
		return -1;
	else
	 	return timer->min.second;
}

void	PollSet::dropTimeout()
{
	vector<int>::iterator it	= timer->timeoutPool.begin();
	vector<int>::iterator ite	= timer->timeoutPool.end();

	for (; it < ite; it++)
	{
		//IMPL: reset_timedout_connection
		// struct linger l = {.l_onoff = 1, .l_linger = 0};
		// setsockopt((*itStream)->getFD(), SOL_SOCKET, SO_LINGER, &l, sizeof(l));

		ConnSocket* c = CONVERT(pollMap[*it].second, ConnSocket);
		FileStream* f = CONVERT(pollMap[*it].second, FileStream);
		Pipe*		p = CONVERT(pollMap[*it].second, Pipe);
		if		(c)		c->unlinkAll();
		else if (f)		(f->linkConn) ? f->linkConn->unlink(f) : (void)0;
		else if (p)		(p->linkConn) ? p->linkConn->unlink(p) : (void)0;

		POLLSET->drop(*it);
	}
	timer->timeoutPool.clear();
}

