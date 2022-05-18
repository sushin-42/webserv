/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Poll.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mishin <mishin@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/10 16:56:25 by mishin            #+#    #+#             */
/*   Updated: 2022/05/18 18:43:12 by mishin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
# include "ResBody.hpp"
# include "ServerSocket.hpp"
# include "color.hpp"




typedef struct pollfd Poll;
class PollSet: private vector<Poll>
{
private:
	typedef vector<Poll>		_Vp;

public:
	typedef _Vp::iterator		iterator;
	using _Vp::begin;

	PollSet(): vector() {}
	PollSet( const PollSet& src ): vector(src) {}
	~PollSet() {}

	PollSet&	operator=( const PollSet& src )
	{
		if (this != &src)
			this->assign(src.begin(), src.end());
		return *this;
	}

	void	enroll( ISocket* sock )
	{
		Poll		p;
		p.fd		= sock->getFD();
		p.events	= POLLIN;
		p.revents	= 0;

		this->push_back(p);
	}

	void	drop( iterator it )
	{
		TAG(PollSet, drop); cout << GRAY("Drop ") << it->fd << endl;
		this->erase(it);
	}

	iterator	examine( const ServerSocket& serv)
	{
		int	numReady = 0;

		// TAG(PollSet, examine); this->print();

		switch (numReady = ::poll(this->data(), this->size(), -1/*time-out*/))
		{
		case -1: TAG(PollSet, examine); cerr << RED("poll() ERROR: ")	<< strerror(errno) << endl;	break;
		case  0: TAG(PollSet, examine); cerr << RED("poll() TIMEOUT")	<< endl;					break;
		default:;
		}

		for (iterator it = this->begin(); it<this->end(); it++)
		{
			if		(it->revents == 0)			continue;
			else if (it->revents & POLLIN)		return readRoutine(it, serv);
			else if (it->revents & POLLOUT)		return writeRoutine(it);
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
			cout << it->fd << " ( ";
			if (it->events & POLLIN) cout << "IN ";
			if (it->events & POLLOUT) cout << "OUT ";
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
					delete connected;
				}
				catch (exception& e)	// accept() not ready
				{	continue;	}

				return (this->begin());
			}
		}
		else
		{
			TAG(PollSet, examine); cout << GREEN("New data to read on ")  << it->fd << endl;
			return it;
		}
	}

	iterator	writeRoutine(iterator it)
	{
		TAG(PollSet, examine); cout << GREEN("Can write to ") << it->fd << endl;
		return it;
	}
};
#endif
