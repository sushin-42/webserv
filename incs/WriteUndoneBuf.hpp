#ifndef WRITEUNDONEBUF_HPP
# define WRITEUNDONEBUF_HPP
#include "utils.hpp"
# include <map>
# include <string>
# define UNDONEBUF	WriteUndoneBuf::_()

using namespace std;

struct undone
{
	string	content;
	ssize_t	totalWrited;
};

class WriteUndoneBuf
{
/**========================================================================
* '                              typedefs
*========================================================================**/
	typedef map<int, struct undone>	_Map;
/**========================================================================
* %                          member variables
*========================================================================**/
	static WriteUndoneBuf*	wub;
	_Map					m;
/**========================================================================
* @                           Constructors
*========================================================================**/

private:
	WriteUndoneBuf();
public:
	~WriteUndoneBuf();

/**========================================================================
* *                            operators
*========================================================================**/

	struct undone&			operator[](int fd);
	const struct undone&	operator[](int fd) const;

/**========================================================================
* #                          member functions
*========================================================================**/
public:
	static WriteUndoneBuf*	_()
	{
		if (wub == NULL) wub = new WriteUndoneBuf;
		return wub;
	}

	const struct undone& 	at(int fd) const;
	void					erase(int fd);
	_Map::iterator			begin();
	_Map::iterator			end();


};

# endif
