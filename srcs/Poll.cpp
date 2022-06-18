#include "Poll.hpp"
#include "Timer.hpp"


/**========================================================================
* @                           Constructors
*========================================================================**/
PollSet::PollSet(): pollVec(), streamVec(), timer(NULL) {}
PollSet::PollSet( const PollSet& src ): pollVec(src.pollVec), streamVec(src.streamVec), timer(NULL) {}
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

PollSet::iterator		PollSet::enroll( IStream* stream )
{
	Poll		p;
	p.fd		= stream->getFD();
	p.events	= POLLIN;
	p.revents	= 0;

	stream->setTimeOut(20);
	stream->updateLastActive();

	pollVec.push_back(p);
	streamVec.push_back(stream);


	ServerSocket*	serv		= CONVERT(stream, ServerSocket);
	ConnSocket*		connected	= CONVERT(stream, ConnSocket);
	Pipe*			P			= CONVERT(stream, Pipe);

	TAG(PollSet, enroll); cout << GRAY("Enroll ") << stream->getFD()
	<< (serv		? CYAN( " (ServerSocket) ") "[" _UL + serv->getIP() + ":" + toString(serv->getPort()) + _NC "]" :
		connected	? BLUE( " (ConnSocket)") :
		P			? PURPLE( " (Pipe)") : "") << endl;

	return make_iterator_pair(pollVec.end()-1, streamVec.end()-1);
}

void	PollSet::drop( PollSet::iterator it )
{
	ConnSocket* connSock = CONVERT(*(it.second), ConnSocket);
	Pipe*			link = NULL;


	if (connSock && connSock->linkReadPipe)
		link = connSock->linkReadPipe;

	TAG(PollSet, drop); cout << GRAY("Drop ") << it.first->fd << endl;
	delete (*it.second);
	pollVec.erase(it.first);
	streamVec.erase(it.second);


	if (link)	/* if disconnected by client and if it was CGI, child process and Pipe still alive.
					we will drop iterator of Pipe and kill child process here.	*/
	{
		Poll			p;
		p.fd = link->getFD();

		iterator_p		itPoll;
		iterator_s		itPipe;

		itPipe = find(streamVec.begin(), streamVec.end(), link);
		itPoll = find(pollVec.begin(), pollVec.end(), p);

		TAG(PollSet, drop); cout << GRAY("Destroy linked pipe ") << (*itPipe)->getFD() << endl;
		kill(link->pid, SIGKILL);
		//NOTE: cannot waitpid() here due to delay. we will check every child process later, because we don't want looping or blocking operation.
		(*itPipe)->close();
		delete (*itPipe);
		pollVec.erase(itPoll);
		streamVec.erase(itPipe);
	}
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
				this->enroll(connected);
			}
			catch (exception& e)	// accept() not ready
			{	continue;	}

			return (make_iterator_pair(
										find(pollVec.begin(), pollVec.end(), p),
										find(streamVec.begin(), streamVec.end(), serv))
									);
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


void	PollSet::createMonitor() { this->timer = new Timer(this); }
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

	PollSet::iterator_p itPoll;
	PollSet::iterator_s itStream;

	for (; it < ite; it++)
	{
		itPoll = find(pollVec.begin(), pollVec.end(), it->first);
		itStream = find(streamVec.begin(), streamVec.end(), it->second);

		//IMPL: reset_timedout_connection
		// struct linger l = {.l_onoff = 1, .l_linger = 0};
		// setsockopt((*itStream)->getFD(), SOL_SOCKET, SO_LINGER, &l, sizeof(l));
		drop(make_iterator_pair(itPoll, itStream));
	}
	timer->timeoutPool.clear();
	// print();
}

