#ifndef TIMER_HPP
# define TIMER_HPP

# include "Poll.hpp"
class Timer
{
/**========================================================================
* '                              typedefs
*========================================================================**/

private:
	typedef PollSet::iterator		iterator;
	typedef PollSet::const_iterator	const_iterator;
	typedef pair<Poll, Stream*>	_Ps;

/**========================================================================
* %                          member variables
*========================================================================**/

	PollSet* pollset;

public:
	vector<_Ps>		timeoutPool;
	pair<_Ps, time_t>	min;

/**========================================================================
* @                           Constructors
*========================================================================**/

	Timer()						: pollset(NULL), timeoutPool(), min(make_pair(Poll(), (Stream*)NULL), numeric_limits<time_t>::max())	{}
	Timer(PollSet* p) 			: pollset(p), timeoutPool(), min(make_pair(Poll(), (Stream*)NULL), numeric_limits<time_t>::max())		{}
	Timer( const Timer& src )	: pollset(src.pollset), timeoutPool(src.timeoutPool), min(src.min)	{}
	~Timer() {};


/**========================================================================
* *                            operators
*========================================================================**/

	Timer&	operator=( const Timer& src )
	{
		if (this != &src)
		{
			this->pollset = src.pollset;
		}
		return *this;
	}

/**========================================================================
* #                          member functions
*========================================================================**/

	void	subscribe(PollSet* p);
	void	monitor();
};

#endif
