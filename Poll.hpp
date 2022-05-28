#ifndef POLL_HPP
# define POLL_HPP
# include <poll.h>
# include <sys/_types/_size_t.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
# include <vector>

# include "ConnSocket.hpp"
# include "ISocket.hpp"
#include "IStream.hpp"
# include "Pipe.hpp"
# include "iterator_pair.hpp"
# include "ResBody.hpp"
# include "ServerSocket.hpp"
# include "color.hpp"



typedef struct pollfd Poll;
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

		ServerSocket*	serv		= dynamic_cast<ServerSocket*>((stream));
		ConnSocket*		connected	= dynamic_cast<ConnSocket*>((stream));
		Pipe*			P			= dynamic_cast<Pipe*>((stream));

		TAG(PollSet, enroll); cout << GRAY("Enroll ") << stream->getFD()
		<< (serv		? CYAN( " (ServerSocket)") :
			connected	? BLUE( " (ConnSocket)") :
			P			? PURPLE( " (Pipe)") : "") << endl;
	}

	void	drop( iterator it )
	{
		TAG(PollSet, drop); cout << GRAY("Drop ") << it.first->fd << endl;

		delete (*it.second);
		pollVec.erase(it.first);
		streamVec.erase(it.second);
	}

	iterator	examine()
	{
		int	numReady = 0;

		// TAG(PollSet, examine); this->print();

		switch (numReady = ::poll(pollVec.data(), pollVec.size(), -1/*time-out*/))
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
		ServerSocket*	serv		= dynamic_cast<ServerSocket*>(*(it.second));
		ConnSocket*		connected	= dynamic_cast<ConnSocket*>(*(it.second));
		Pipe*			P			= dynamic_cast<Pipe*>(*(it.second));

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

				return (it);
			}
		}
		else if (connected)
		{
			TAG(PollSet, examine); cout << GREEN("New data to read on ")  << it.first->fd << endl;
			return it;
		}
		else
		{
			(void)P;
			cout << "It is a pipe." << endl;
			return it;
		}
	}

	iterator	writeRoutine(iterator it)
	{
		TAG(PollSet, examine); cout << GREEN("Can write to ") << it.first->fd << endl;
		return it;
	}
};
#endif
