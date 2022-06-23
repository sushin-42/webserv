#include "Poll.hpp"
#include "FileStream.hpp"
#include "Pipe.hpp"
#include "Timer.hpp"
#include <ios>

PollSet*	PollSet::pollset;

/**========================================================================
* @                           Constructors
*========================================================================**/

PollSet::PollSet(): pollVec(), streamVec(), timer(NULL) { }
PollSet::~PollSet() {}

/**========================================================================
* *                            operators
*========================================================================**/
// PollSet&	PollSet::operator=( const PollSet& src )
// {
// 	if (this != &src)
// 	{
// 		pollVec.assign(src.begin().first, src.end().first);
// 		streamVec.assign(src.begin().second, src.end().second);
// 	}
// 	return *this;
// }

/**========================================================================
* #                          member functions
*========================================================================**/

PollSet::iterator		PollSet::begin()			{ return make_iterator_pair(pollVec.begin(), streamVec.begin()); }
PollSet::iterator		PollSet::end()			{ return make_iterator_pair(pollVec.end(), streamVec.end()); }
PollSet::const_iterator	PollSet::begin() const	{ return make_iterator_pair(pollVec.begin(), streamVec.begin()); }
PollSet::const_iterator	PollSet::end() const		{ return make_iterator_pair(pollVec.end(), streamVec.end()); }


PollSet::iterator	PollSet::getIterator(Stream* s)
{
	iterator_p				itPoll;
	iterator_s				itStream;
	_Vs::difference_type	distance;

	itStream = find(streamVec.begin(), streamVec.end(), s);
	distance = itStream - streamVec.begin();
	itPoll = pollVec.begin() + distance;

	return iterator(itPoll, itStream);
}

PollSet::iterator		PollSet::enroll( Stream* stream, short events )
{
	Poll		p;
	p.fd		= stream->getFD();
	p.events	= events;
	p.revents	= 0;

	stream->setTimeOut(20);
	stream->updateLastActive();

	pollVec.push_back(p);
	streamVec.push_back(stream);

	ServerSocket*	serv		= CONVERT(stream, ServerSocket);
	ConnSocket*		connected	= CONVERT(stream, ConnSocket);
	FileStream*		filestream	= CONVERT(stream, FileStream);
	Pipe*			P			= CONVERT(stream, Pipe);

	TAG(PollSet, enroll); cout << GRAY("Enroll ") << stream->getFD()
	<< (serv		? CYAN( " (ServerSocket) ") "[" _UL + serv->getIP() + ":" + toString(serv->getPort()) + _NC "]" :
		connected	? BLUE( " (ConnSocket)") :
		filestream	? YELLOW( " (FileStream) ") :
		P			? PURPLE( " (Pipe)") : "") << endl;

	return make_iterator_pair(pollVec.end()-1, streamVec.end()-1);
}


void	PollSet::dropLink(Stream* link)
{
	Pipe*			linkPipe = CONVERT(link, Pipe);
	FileStream*		linkFile = CONVERT(link, FileStream);

	iterator		it = getIterator(link);
	iterator_p		itPoll = it.first;
	iterator_s		itLink = it.second;

	if (linkFile)	{ TAG(PollSet, dropLink); cout << GRAY("Destroy linked File ") << (*itLink)->getFD() << endl; }
	if (linkPipe)	{ TAG(PollSet, dropLink); cout << GRAY("Destroy linked pipe ") << (*itLink)->getFD() << endl; kill(linkPipe->pid, SIGKILL); }	//NOTE: cannot waitpid() here due to delay. we will check every child process later, because we don't want looping or blocking operation.

	(*itLink)->close();
	delete (*itLink);
	pollVec.erase(itPoll);
	streamVec.erase(itLink);
}

void	PollSet::drop( PollSet::iterator it )
{
	ConnSocket* connSock = CONVERT(*(it.second), ConnSocket);
	Stream*	link1 = NULL;
	Stream*	link2 = NULL;
	Stream*	link3 = NULL;
	Stream*	link4 = NULL;

	if (connSock && connSock->linkInputPipe)
		link1 = connSock->linkInputPipe;
	if (connSock && connSock->linkOutputPipe)
		link2 = connSock->linkOutputPipe;
	if (connSock && connSock->linkInputFile)
		link3 = connSock->linkInputFile;
	if (connSock && connSock->linkOutputFile)
		link4 = connSock->linkOutputFile;

	TAG(PollSet, drop); cout << GRAY("Drop ") << it.first->fd << endl;
	delete (*it.second);
	pollVec.erase(it.first);
	streamVec.erase(it.second);

	if (link1)	{/* cout << "drop link 1 " << endl; */dropLink(link1);}
	if (link2)	{/* cout << "drop link 2 " << endl; */dropLink(link2);}
	if (link3)	{/* cout << "drop link 3 " << endl; */dropLink(link3);}
	if (link4)	{/* cout << "drop link 4 " << endl; */dropLink(link4);}

}

PollSet::iterator	PollSet::examine()
{
	int		numReady = 0;
	time_t	minRemaining = 2000;;
	if (timer)
	{
		if ((minRemaining = getMinimumRemaining() * 1000) < 0)
			minRemaining = 2000;
		dropTimeout();
	}
	switch (numReady = ::poll(pollVec.data(), pollVec.size(), minRemaining/*time-out*/))
	{
	case -1: TAG(PollSet, examine); cerr << RED("poll() ERROR: ")	<< strerror(errno) << endl;	break;
	case  0: TAG(PollSet, examine); cerr << GRAY("No event within ") << minRemaining << "ms" << endl;					break;
	default:;
	}

	for (iterator it = this->begin(); it<this->end(); it++)
	{
		if		(it.first->revents == 0)			continue;
		else
		{
			//IMPL: separate updateLastWrite, updateLastRead for send_timeout
			(*it.second)->updateLastActive();
			if (it.first->revents & POLLIN)			return readRoutine(it);
			if (it.first->revents & POLLOUT)		return writeRoutine(it);
		}
	}
	throw exception();
}

void	PollSet::print()
{
	int i = 0;
	for (iterator it = this->begin(); it<this->end(); it++, i++)
	{
		cout << "[" << i << "] ";
		cout << it.first->fd << " ( ";
		if (it.first->events & POLLIN) cout << "IN ";
		if (it.first->events & POLLOUT) cout << "OUT ";
		cout << ") ";
	}
	cout << endl;
}

PollSet::iterator	PollSet::readRoutine(PollSet::iterator it)
{
	ServerSocket*	serv		= CONVERT(*(it.second),ServerSocket);
	ConnSocket*		connected	= CONVERT(*(it.second),ConnSocket);
	Pipe*			CGIpipe		= CONVERT(*(it.second),Pipe);
	FileStream*		filestream	= CONVERT(*(it.second),FileStream);

	if (serv)
	{
		Poll p;
		p.fd = serv->getFD();
		while(1)	// accept() will throw exception if not readied
		{
			try
			{
				connected = new ConnSocket(serv->accept());
				TAG(PollSet, examine); cout << GREEN("Server ") << _UL "[" << serv->getIP() + ":" + toString(serv->getPort()) + _NC "]"<<  GREEN(" Got new connection, enroll ") << connected->getFD() << endl;
				this->enroll(connected, POLLIN);
			}
			catch (exception& e)	// accept() not ready
			{	continue;	}

			return (getIterator(serv));
		}
	}
	else if (connected)
	{
		TAG(PollSet, examine); cout << GREEN("New data to read ")
		<< it.first->fd << BLUE(" (ConnSocket)") <<endl;
		return it;
	}
	else if (CGIpipe)
	{
		TAG(PollSet, examine); cout << GREEN("New data to read ")
		<< it.first->fd << PURPLE(" (Pipe)") <<endl;
		return it;
	}
	else if (filestream)
	{
		TAG(PollSet, examine); cout << GREEN("New data to read ")
		<< it.first->fd << YELLOW(" (FileStream)") <<endl;
		return it;
	}
	else
	{
		TAG(PollSet, examine); cout << RED("Unknown type: ") << endl;
		throw exception();
	}
}

PollSet::iterator	PollSet::writeRoutine(iterator it)
{
	TAG(PollSet, examine); cout << GREEN("Can write to ") << it.first->fd << endl;
	return it;
}


void	PollSet::createMonitor() { this->timer = new Timer(); }
time_t	PollSet::getMinimumRemaining()
{
	timer->monitor();
	if (timer->min.second == numeric_limits<time_t>::max())
		return -1;
	else
	 	return timer->min.second;
}

void	PollSet::dropTimeout()
{
	vector<_Ps>::iterator it	= timer->timeoutPool.begin();
	vector<_Ps>::iterator ite	= timer->timeoutPool.end();

	iterator target;
	for (; it < ite; it++)
	{
		//IMPL: reset_timedout_connection
		// struct linger l = {.l_onoff = 1, .l_linger = 0};
		// setsockopt((*itStream)->getFD(), SOL_SOCKET, SO_LINGER, &l, sizeof(l));

		target = getIterator(it->second);

		if (target.second == streamVec.end())	// already removed by dropLink()
			continue;
		else
			drop(target);
	}
	timer->timeoutPool.clear();
}

