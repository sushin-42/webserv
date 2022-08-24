#ifndef RESBODY_HPP
# define RESBODY_HPP

#include "FileStream.hpp"
#include "AText.hpp"
#include "color.hpp"
#include <iostream>
# include <string>
#include <sys/errno.h>
# include <sys/fcntl.h>
# include <unistd.h>
# include <fstream>
using namespace std;

typedef unsigned short status_code_t;

class ResBody : public AText
{
public:

/**========================================================================
* @                           Constructors
*========================================================================**/

	ResBody(): AText() {}
	ResBody( const ResBody& src ): AText(src.content) {}
	~ResBody() {}

/**========================================================================
* *                            operators
*========================================================================**/

	ResBody&	operator=( const ResBody& src );

/**========================================================================
* #                          member functions
*========================================================================**/

	void	clear();
};
#endif
