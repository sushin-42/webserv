#ifndef RESBODY_HPP
# define RESBODY_HPP

#include "FileStream.hpp"
#include "IText.hpp"
#include "color.hpp"
#include <iostream>
# include <string>
#include <sys/errno.h>
# include <sys/fcntl.h>
# include <unistd.h>
# include <fstream>
using namespace std;

typedef unsigned short status_code_t;

class ResBody : public IText
{
public:

/**========================================================================
* @                           Constructors
*========================================================================**/

	ResBody(): IText() {}
	ResBody( const string& path ): IText()
	{
		readFile(path);
	}
	ResBody( const ResBody& src ): IText(src.content) {}
	~ResBody() {}

/**========================================================================
* *                            operators
*========================================================================**/

	ResBody&	operator=( const ResBody& src );

/**========================================================================
* #                          member functions
*========================================================================**/

	status_code_t	readFile( FileStream* f );
	status_code_t	readFile( const string& path );
	void	clear();
};
#endif
