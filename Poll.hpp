#ifndef POLL_HPP
# define POLL_HPP
#include <csignal>
#include <exception>
# include <poll.h>
# include <sys/_types/_size_t.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
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
private:
	typedef vector<Poll>			_Vp;
	typedef vector<IStream*>		_Vs;
	typedef _Vp::iterator			iterator_p;
	typedef _Vs::iterator			iterator_s;
	typedef _Vp::const_iterator		const_iterator_p;
	typedef _Vs::const_iterator		const_iterator_s;

	vector<Poll>		pollVec;
	vector<IStream*>	streamVec;
	timeVal				timeVal;

public:

	typedef iterator_pair<iterator_p, iterator_s>				iterator;
	typedef iterator_pair<const_iterator_p,const_iterator_s>	const_iterator;


	PollSet(): pollVec(), streamVec() {}
	PollSet( const PollSet& src ): pollVec(src.pollVec), streamVec(src.streamVec) {}
	~PollSet() {}

	PollSet&	operator=( const PollSet& src )
	{
		if (this != &src)
		{
			pollVec.assign(src.begin().first, src.end().first);
			streamVec.assign(src.begin().second, src.end().second);
		}
		return *this;
	}

	iterator		begin()			{ return make_iterator_pair(pollVec.begin(), streamVec.begin()); }
	iterator		end()			{ return make_iterator_pair(pollVec.end(), streamVec.end()); }
	const_iterator	begin() const	{ return make_iterator_pair(pollVec.begin(), streamVec.begin()); }
	const_iterator	end() const		{ return make_iterator_pair(pollVec.end(), streamVec.end()); }

	void	enroll( IStream* stream )
	{
		Poll		p;
		p.fd		= stream->getFD();
		p.events	= POLLIN;
		p.revents	= 0;

		pollVec.push_back(p);
		streamVec.push_back(stream);

		ServerSocket*	serv		= CONVERT(stream, ServerSocket);
		ConnSocket*		connected	= CONVERT(stream, ConnSocket);
		Pipe*			P			= CONVERT(stream, Pipe);

		TAG(PollSet, enroll); cout << GRAY("Enroll ") << stream->getFD()
		<< (serv		? CYAN( " (ServerSocket)") :
			connected	? BLUE( " (ConnSocket)") :
			P			? PURPLE( " (Pipe)") : "") << endl;
	}

	void	drop( iterator it )
	{
		ConnSocket* connSock = CONVERT(*(it.second), ConnSocket);
		Pipe*			link = NULL;


		if (connSock && connSock->linkPipe)
			link = connSock->linkPipe;

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

	iterator	examine()
	{
		int	numReady = 0;
		// TAG(PollSet, examine); this->print();

		pthread_mutex_lock(&(timeVal.timerLock));
		if (timeVal.iStreamAddr.size() != 0)
		{
			for (vector<IStream *>::iterator addrIter = timeVal.iStreamAddr.begin(); addrIter != timeVal.iStreamAddr.end(); addrIter++)
			{
				for (iterator iter = this->begin(); iter != this->end(); iter++)
				{
					if (*addrIter == *(iter.second))
					{
						(*addrIter)->close();
						drop(iter);
						break;
					}
				}
			}
			timeVal.iStreamAddr.clear();
		}
		pthread_mutex_unlock(&(timeVal.timerLock));
		// 타이머 테스트를 위해 폴을 3초에 한번씩 빠져나가도록 설정
		switch (numReady = ::poll(pollVec.data(), pollVec.size(), 3000/*time-out*/))
		{
		case -1: TAG(PollSet, examine); cerr << RED("poll() ERROR: ")	<< strerror(errno) << endl;	break;
		case  0: TAG(PollSet, examine); cerr << RED("poll() TIMEOUT")	<< endl;					break;
		default:;
		}

		for (iterator it = this->begin(); it<this->end(); it++)
		{
			if		(it.first->revents == 0)			continue;
			else if (it.first->revents & POLLIN)		return readRoutine(it);
			else if (it.first->revents & POLLOUT)		return writeRoutine(it);
		}
		throw exception();
	}

	void	startTimer()
	{
		timeVal.iStream = &(this->streamVec);
		pthread_mutex_init(&(timeVal.timerLock),NULL);
		pthread_create(&timeVal.timerThread, NULL, timer, (void *)&timeVal);
	}

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
#endif
