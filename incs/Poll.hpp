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

#include "Config.hpp"
# include "ConnSocket.hpp"
# include "ISocket.hpp"
#include "Stream.hpp"
# include "Pipe.hpp"
# include "FileStream.hpp"
# include "iterator_pair.hpp"
# include "ResBody.hpp"
# include "ServerSocket.hpp"
# include "color.hpp"
# include "utils.hpp"

# define POLLSET	PollSet::_()
class Timer;
class Poll : public pollfd
{
};

class PollSet

{
/**========================================================================
* '                              typedefs
*========================================================================**/

typedef pair<Poll, Stream*> _PS;
typedef vector<Poll>		_Vp;
typedef map<int, _PS>		_Map;


/**========================================================================
* %                          member variables
*========================================================================**/

private:
	static PollSet*		pollset;
	_Vp					pollVec;

public:
	_Map				pollMap;
private:
	Timer*				timer;
/**========================================================================
* @                           Constructors
*========================================================================**/

private:
	PollSet();
public:
	~PollSet();

/**========================================================================
* #                          member functions
*========================================================================**/
public:
	static PollSet* _()
	{
		if (pollset == NULL)
		{
			pollset = new PollSet;
		}
		return pollset;
	}


	void			enroll( Stream* stream, short events );
	void			dropLink(Stream* link);
	void			drop( int fd );
	void			drop( Stream* stream );

	time_t		getMinimumRemaining();
	void		dropTimeout();
	vector<Stream*>	examine();
	void		createMonitor();
	// iterator	getIterator(Stream* s);
private:
	void			print();
	void			makePollVec();
	void			_drop( int fd );

	Stream*	readRoutine(Stream* stream);
	Stream*	writeRoutine(Stream* stream);


};
#endif
