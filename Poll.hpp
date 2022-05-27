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
# include "iterator_pair.hpp"
# include "ResBody.hpp"
# include "ServerSocket.hpp"
# include "color.hpp"



typedef struct pollfd Poll;
class PollSet
{
private:
	typedef vector<Poll>			_Vp;
	typedef vector<ISocket*>		_Vs;
	typedef _Vp::iterator			iterator_p;
	typedef _Vs::iterator			iterator_s;
	typedef _Vp::const_iterator		const_iterator_p;
	typedef _Vs::const_iterator		const_iterator_s;

	vector<Poll>		pollVec;
	vector<ISocket*>	sockVec;

public:

	typedef iterator_pair<iterator_p, iterator_s>				iterator;
	typedef iterator_pair<const_iterator_p,const_iterator_s>	const_iterator;


	PollSet(): pollVec(), sockVec() {}
	PollSet( const PollSet& src ): pollVec(src.pollVec), sockVec(src.sockVec) {}
	~PollSet() {}

	PollSet&	operator=( const PollSet& src )
	{
		if (this != &src)
		{
			pollVec.assign(src.begin().first, src.end().first);
			sockVec.assign(src.begin().second, src.end().second);
		}
		return *this;
	}

	iterator		begin()			{ return make_iterator_pair(pollVec.begin(), sockVec.begin()); }
	iterator		end()			{ return make_iterator_pair(pollVec.end(), sockVec.end()); }
	const_iterator	begin() const	{ return make_iterator_pair(pollVec.begin(), sockVec.begin()); }
	const_iterator	end() const		{ return make_iterator_pair(pollVec.end(), sockVec.end()); }

	void	enroll( ISocket* sock )
	{
		Poll		p;
		p.fd		= sock->getFD();
		p.events	= POLLIN;
		p.revents	= 0;

		pollVec.push_back(p);
		sockVec.push_back(sock);
	}

	void	drop( iterator it )
	{
		TAG(PollSet, drop); cout << GRAY("Drop ") << it.first->fd << endl;

		delete (*it.second);
		pollVec.erase(it.first);
		sockVec.erase(it.second);


	}

	iterator	examine( const ServerSocket& serv)
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
			else if (it.first->revents & POLLIN)		return readRoutine(it, serv);
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

	iterator	readRoutine(iterator it, const ServerSocket& serv)
	{
		ConnSocket* connected = NULL;

		if (it == this->begin())
		{
			while(1)	// accept() will throw exception if not readied
			{
				try
				{
					connected = new ConnSocket(serv.accept());
					this->enroll(connected);
					TAG(PollSet, examine); cout << GREEN("Got new connection, enroll ") << connected->getFD() << endl;
				}
				catch (exception& e)	// accept() not ready
				{	continue;	}

				return (this->begin());
			}
		}
		else
		{
			TAG(PollSet, examine); cout << GREEN("New data to read on ")  << it.first->fd << endl;
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
