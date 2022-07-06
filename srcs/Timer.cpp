#include "Timer.hpp"


/**========================================================================
* @                           Constructors
*========================================================================**/

Timer::Timer()
: timeoutPool(), min(-1, numeric_limits<time_t>::max())	{}
Timer::~Timer() {}

/**========================================================================
* #                          member functions
*========================================================================**/
void	Timer::monitor()
{
	PollSet::_Map::iterator it = POLLSET->pollMap.begin();
	PollSet::_Map::iterator ite = POLLSET->pollMap.end();

	pair<int, time_t> min = make_pair(-1, numeric_limits<time_t>::max() );

	time_t	now = time(NULL);
	time_t	timeout = -1;
	time_t	lastActive = -1;

	time_t	elapsedTime;
	time_t	remainingTime;
	Stream* stream;

	for (;it != ite; it++)
	{
		stream = it->second.second;
		if (CONVERT(stream, ServerSocket))
			continue ;
		if ((timeout = (stream)->getTimeOut()) < 0)	// No limit
			continue ;
		if ((lastActive = (stream)->getLastActive()) < 0)
			continue ;

		/*--------------------------------------------*/
		elapsedTime = difftime(now, lastActive);
		remainingTime = difftime(timeout, elapsedTime);
		if (remainingTime <= 0)
		{
			LOGGING(Timer,  UL("%d") RED( "TIMEOUT %lus"), stream->getFD(), timeout);
			timeoutPool.push_back(stream->getFD());
		}
		else if (remainingTime <= min.second)
		{
			min.first = stream->getFD();
			min.second = remainingTime;
		}
	}
	this->min = min;
}
