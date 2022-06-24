// #include "Timer.hpp"


// /**========================================================================
// * @                           Constructors
// *========================================================================**/

// Timer::Timer()
// : timeoutPool(), min(make_pair(Poll(), (Stream*)NULL), numeric_limits<time_t>::max())	{}
// Timer::~Timer() {}

// /**========================================================================
// * #                          member functions
// *========================================================================**/
// 	void	Timer::monitor()
// 	{
// 		iterator it = POLLSET->begin();
// 		iterator ite = POLLSET->end();

// 		pair<_Ps, time_t> min = make_pair(
// 									make_pair(*it.first, *it.second),
// 									numeric_limits<time_t>::max()
// 								);

// 		time_t	now = time(NULL);
// 		time_t	timeout = -1;
// 		time_t	lastActive = -1;

// 		time_t	elapsedTime;
// 		time_t	remainingTime;

// 		for (;it < ite; it++)
// 		{
// 			if (CONVERT(*it.second, ServerSocket))
// 				continue ;
// 			if ((timeout = (*it.second)->getTimeOut()) < 0)	// No limit
// 				continue ;
// 			if ((lastActive = (*it.second)->getLastActive()) < 0)
// 				continue ;

// 			/*--------------------------------------------*/
// 			elapsedTime = difftime(now, lastActive);
// 			remainingTime = difftime(timeout, elapsedTime);
// 			if (remainingTime <= 0)
// 			{
// 				TAG(Timer, monitor) << _UL << it.first->fd << _NC << RED(" TIMEOUT: ") _UL << timeout  << "s" << _NC <<  endl;
// 				timeoutPool.push_back(make_pair(*it.first, *it.second));
// 			}
// 			else if (remainingTime <= min.second)
// 			{
// 				min.first = make_pair(*it.first, *it.second);
// 				min.second = remainingTime;
// 			}
// 		}
// 		this->min = min;
// 	}
