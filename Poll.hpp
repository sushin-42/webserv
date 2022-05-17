/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Poll.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mishin <mishin@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/10 16:56:25 by mishin            #+#    #+#             */
/*   Updated: 2022/05/17 18:55:32 by mishin           ###   ########.fr       */
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

		cout << "enroll [" << p.fd << "] to pollset" <<endl;
		this->push_back(p);
	}

	void	drop( iterator it )
	{
		cout << "drop" <<endl;
		this->erase(it);
	}

	_Vp::iterator	examine( const ServerSocket& serv)
	{
		ConnSocket* 	connected = NULL;
		int				numReady = 0;

		switch (numReady = ::poll(this->data(), this->size(), -1/*time-out*/))
		{
		case -1: cerr << RED("poll() ERROR: ")	<< strerror(errno) << endl;	break;
		case 0 : cerr << RED("poll() TIMEOUT")	<< endl;					break;
		default:;
		}

		for (_Vp::iterator it = this->begin(); it<this->end(); it++)
		{
			if (it->revents == 0)		continue;
			if (!(it->revents & POLLIN)) { cout << it->revents << endl; exit(123); }

			if (it == this->begin())
			{
				while(1)	// accept() will throw exception if not readied
				{
					try
					{
						connected = new ConnSocket(serv.accept());
						this->enroll(connected);

						cout << GREEN("got new connection") << endl;
						delete connected;
					}
					catch (exception& e) { continue; }	// accept() not ready
					throw exception();	// enroll OK, Cannot return NULL :(
				}
			}
			else
			{
				cout << GREEN("new to read")  " => " << it->fd << endl;
				return it;
			}
		}
		throw exception();
		}
};
#endif
