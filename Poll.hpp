#ifndef POLL_HPP
# define POLL_HPP
#include <csignal>
#include <ctime>
#include <exception>
#include <limits>
# include <poll.h>
# include <sys/_types/_size_t.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>
# include <vector>
# include <pthread.h>

# include "ConnSocket.hpp"
# include "ISocket.hpp"
#include "IStream.hpp"
# include "Pipe.hpp"
# include "iterator_pair.hpp"
# include "ResBody.hpp"
# include "ServerSocket.hpp"
# include "color.hpp"
# include "utils.hpp"

class Timer;

class Poll : public pollfd
{
public:
	bool operator==(const Poll& p)
	{
		return (
			this->fd == p.fd
		);
	}
};

class PollSet

{
/**========================================================================
* '                              typedefs
*========================================================================**/

private:
	typedef vector<Poll>			_Vp;
	typedef vector<IStream*>		_Vs;
	typedef pair<Poll, IStream*>	_Ps;
	typedef _Vp::iterator			iterator_p;
	typedef _Vs::iterator			iterator_s;
	typedef _Vp::const_iterator		const_iterator_p;
	typedef _Vs::const_iterator		const_iterator_s;
public:
	typedef iterator_pair<iterator_p, iterator_s>				iterator;
	typedef iterator_pair<const_iterator_p,const_iterator_s>	const_iterator;

/**========================================================================
* %                          member variables
*========================================================================**/

private:
	vector<Poll>		pollVec;
	vector<IStream*>	streamVec;
	Timer*				timer;

/**========================================================================
* @                           Constructors
*========================================================================**/
public:
	PollSet(): pollVec(), streamVec(), timer(NULL) {}
	PollSet( const PollSet& src ): pollVec(src.pollVec), streamVec(src.streamVec), timer(NULL) {}
	~PollSet();

/**========================================================================
* *                            operators
*========================================================================**/

	PollSet&	operator=( const PollSet& src )
	{
		if (this != &src)
		{
			pollVec.assign(src.begin().first, src.end().first);
			streamVec.assign(src.begin().second, src.end().second);
		}
		return *this;
	}

/**========================================================================
* #                          member functions
*========================================================================**/

	iterator		begin()			{ return make_iterator_pair(pollVec.begin(), streamVec.begin()); }
	iterator		end()			{ return make_iterator_pair(pollVec.end(), streamVec.end()); }
	const_iterator	begin() const	{ return make_iterator_pair(pollVec.begin(), streamVec.begin()); }
	const_iterator	end() const		{ return make_iterator_pair(pollVec.end(), streamVec.end()); }

	iterator	enroll( IStream* stream )
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
		<< (serv		? CYAN( " (ServerSocket)") :
			connected	? BLUE( " (ConnSocket)") :
			P			? PURPLE( " (Pipe)") : "") << endl;

		return make_iterator_pair(pollVec.end()-1, streamVec.end()-1);
	}

	void	drop( iterator it )
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

	time_t	getMinimumRemaining();
	void	dropTimeout();
	iterator	examine()
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
				(*it.second)->updateLastActive();
				if (it.first->revents & POLLIN)			return readRoutine(it);
				if (it.first->revents & POLLOUT)		return writeRoutine(it);
			}
		}
		throw exception();
	}

	void	createMonitor();
private:
	void	print()
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

	iterator	readRoutine(iterator it)
	{
		ServerSocket*	serv		= CONVERT(*(it.second),ServerSocket);
		ConnSocket*		connected	= CONVERT(*(it.second),ConnSocket);
		Pipe*			CGIpipe		= CONVERT(*(it.second),Pipe);

		if (serv)
		{
			while(1)	// accept() will throw exception if not readied
			{
				try
				{
					connected = new ConnSocket(serv->accept());
					TAG(PollSet, examine); cout << GREEN("Got new connection, enroll ") << connected->getFD() << endl;
					this->enroll(connected);
				}
				catch (exception& e)	// accept() not ready
				{	continue;	}

				return (this->begin());
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

	iterator	writeRoutine(iterator it)
	{
		TAG(PollSet, examine); cout << GREEN("Can write to ") << it.first->fd << endl;
		return it;
	}

};




class Timer
{
/**========================================================================
* '                              typedefs
*========================================================================**/

private:
	typedef PollSet::iterator		iterator;
	typedef PollSet::const_iterator	const_iterator;
	typedef pair<Poll, IStream*>	_Ps;

/**========================================================================
* %                          member variables
*========================================================================**/

	PollSet* pollset;

public:
	vector<_Ps>		timeoutPool;
	pair<_Ps, time_t>	min;

/**========================================================================
* @                           Constructors
*========================================================================**/

	Timer()						: pollset(NULL), timeoutPool(), min(make_pair(Poll(), (IStream*)NULL), numeric_limits<time_t>::max())	{}
	Timer(PollSet* p) 			: pollset(p), timeoutPool(), min(make_pair(Poll(), (IStream*)NULL), numeric_limits<time_t>::max())		{}
	Timer( const Timer& src )	: pollset(src.pollset), timeoutPool(src.timeoutPool), min(src.min)	{}
	~Timer() {};


/**========================================================================
* *                            operators
*========================================================================**/

	Timer&	operator=( const Timer& src )
	{
		if (this != &src)
		{
			this->pollset = src.pollset;
		}
		return *this;
	}

/**========================================================================
* #                          member functions
*========================================================================**/

	void	subscribe(PollSet* p)
	{
		this->pollset = p;
	}

	void	monitor()
	{
		iterator it = pollset->begin();
		iterator ite = pollset->end();

		pair<_Ps, time_t> min = make_pair(
									make_pair(*it.first, *it.second),
									numeric_limits<time_t>::max()
								);

		time_t	now = time(NULL);
		time_t	timeout = -1;
		time_t	lastActive = -1;

		time_t	elapsedTime;
		time_t	remainingTime;

		for (;it < ite; it++)
		{
			if (CONVERT(*it.second, ServerSocket))
				continue ;
			if ((timeout = (*it.second)->getTimeOut()) < 0)	// No limit
				continue ;
			if ((lastActive = (*it.second)->getLastActive()) < 0)
				continue ;

			/*--------------------------------------------*/
			elapsedTime = difftime(now, lastActive);
			remainingTime = difftime(timeout, elapsedTime);
			if (remainingTime <= 0)
			{
				TAG(Timer, monitor) << _UL << it.first->fd << _NC << RED(" TIMEOUT: ") _UL << timeout  << "s" << _NC <<  endl;
				timeoutPool.push_back(make_pair(*it.first, *it.second));
			}
			else if (remainingTime <= min.second)
			{
				min.first = make_pair(*it.first, *it.second);
				min.second = remainingTime;
			}
		}
		this->min = min;
	}
};



void	PollSet::createMonitor() { this->timer = new Timer(this); }

PollSet::~PollSet() {delete timer;}
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

		drop(make_iterator_pair(itPoll, itStream));
	}
	timer->timeoutPool.clear();
	// print();
}


#endif
