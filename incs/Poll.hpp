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
	PollSet();
	PollSet( const PollSet& src );
	~PollSet();

/**========================================================================
* *                            operators
*========================================================================**/

	PollSet&	operator=( const PollSet& src );

/**========================================================================
* #                          member functions
*========================================================================**/

	iterator		begin();
	iterator		end();
	const_iterator	begin() const;
	const_iterator	end() const;

	iterator	enroll( IStream* stream );
	void		drop( iterator it );

	time_t		getMinimumRemaining();
	void		dropTimeout();
	iterator	examine();
	void		createMonitor();
private:
	void	print();

	iterator	readRoutine(iterator it);
	iterator	writeRoutine(iterator it);

};
#endif
