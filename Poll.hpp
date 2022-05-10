/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Poll.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mishin <mishin@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/10 16:56:25 by mishin            #+#    #+#             */
/*   Updated: 2022/05/10 23:33:27 by mishin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef POLL_HPP
# define POLL_HPP
# include <poll.h>
# include <sys/_types/_size_t.h>
# include <vector>

# include "ConnSocket.hpp"
# include "ServerSocket.hpp"
# include "color.hpp"

class Poll : public pollfd
{
friend class PollSet;
// private:
public:
	ISocket* s;

public:
	Poll()
	{
		this->fd		= -1;
		this->events	= POLLIN;
		this->revents	= 0;
		this->s			= NULL;
	}
	Poll( ISocket* S )
	{
		this->fd		= S->getFD();
		this->events	= POLLIN;
		this->revents	= 0;
		// a=b=c=0;
		this->s			= S;
	}
	Poll( const Poll& src )
	{
		this->fd		= src.fd;
		this->events	= src.events;
		this->revents	= src.revents;
		this->s			= src.s;
	}
	~Poll()
	{
		// s->close();
		// delete s;
	}

	Poll&	operator=( const Poll& src )
	{
		if (this != &src)
		{
			this->fd		= src.fd;
			this->events	= src.events;
			this->revents	= src.revents;
			this->s			= src.s;
		}
		return *this;
	}

};

class PollSet
{
private:
	typedef vector<Poll> _Vp;
	_Vp	arr;

public:
	PollSet(): arr() {}
	PollSet( const PollSet& src ): arr(src.arr) {}
	~PollSet() {}

	PollSet&	operator=( const PollSet& src )
	{
		if (this != &src)
			this->arr.assign(src.arr.begin(), src.arr.end());
		return *this;
	}

	void	enroll( const Poll& poll )
	{
		cout << "enroll" <<endl;
		arr.push_back(poll);
	}

	Poll*	examine( const ServerSocket& serv )
	{
		ConnSocket* 	connected = NULL;
		int				numReady;
		_Vp::size_type	size = arr.size();

		if ((numReady = ::poll(arr.data(), size, -1/*time-out*/)) > 0)
		{
			for (_Vp::size_type i = 0; i<size; i++)
			{
				if (arr[i].revents & POLLIN)
				{
					if (arr[i].fd == serv.getFD())
					{
						cout << GREEN("got new connection:") " [" << i << "] " << arr[i].fd << endl;
						connected = new ConnSocket(serv.accept());
						// cout << connected << " : " << connected->getFD() << endl;
						Poll tmp(connected);
						arr.push_back(tmp);
						// cout << tmp.s << " : " << connected->getFD() << endl;
						// cout << arr[size].s << " : " << connected->getFD() << endl;
						cout << YELLOW("poll new socket:")  " [" << size << "] " << arr[size].fd << endl;
						connected = NULL;
						// return &arr[size];
					}
					else
					{
						cout << GREEN("new to read")  " => " << arr[i].fd << endl;
						return &arr[i];
						// ((ConnSocket*)connected)->recvRequest();
					}
				}

			}
		}
		return NULL;
	}

};
#endif
