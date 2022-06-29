#include "ResBody.hpp"
#include "FileStream.hpp"



/**========================================================================
* *                            operators
*========================================================================**/

	ResBody&	ResBody::operator=( const ResBody& src )
	{
		if (this != &src)
			content = src.content;
		return *this;
	}

/**========================================================================
* #                          member functions
*========================================================================**/
	void	ResBody::clear() { content.clear(); /*IText::clear();*/ }

