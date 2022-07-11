#include "WriteUndoneBuf.hpp"

WriteUndoneBuf*	WriteUndoneBuf::wub;

/**========================================================================
* @                           Constructors
*========================================================================**/

WriteUndoneBuf::WriteUndoneBuf() {}
WriteUndoneBuf::~WriteUndoneBuf() {}

/**========================================================================
* #                          member functions
*========================================================================**/

	const struct undone&	WriteUndoneBuf::at(int fd) const		{ return getValueIfExists(m, fd); }
	void					WriteUndoneBuf::erase(int fd)			{ m.erase(fd); }
	WriteUndoneBuf::_Map::iterator			WriteUndoneBuf::begin()	{return m.begin();}
	WriteUndoneBuf::_Map::iterator			WriteUndoneBuf::end()	{return m.end();}

/**========================================================================
* *                            operators
*========================================================================**/

	struct undone&			WriteUndoneBuf::operator[](int fd)			{ return m[fd]; }
	const struct undone&	WriteUndoneBuf::operator[](int fd) const	{ return getValueIfExists(m, fd); }
