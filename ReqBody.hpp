#ifndef REQBODY_HPP
# define REQBODY_HPP

#include "IText.hpp"
#include <iostream>
# include <string>
# include <sys/fcntl.h>
# include <unistd.h>
# include <fstream>
using namespace std;

class ReqBody : public IText
{
public:

	ReqBody(): IText() {}
	ReqBody( const ReqBody& src ): IText(src.content) {}
	~ReqBody() {}

	ReqBody&	operator=( const ReqBody& src )
	{
		if (this != &src)
			content = src.content;
		return *this;
	}

	void	clear() { content.clear(); /*IText::clear();*/ }
};
#endif
