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
	ReqBody( const ReqBody& src ): IText(src.content){}
	~ReqBody() {}

	ReqBody&	operator=( const ReqBody& src )
	{
		if (this != &src)
		{
			content = src.content;
		}
		return *this;
	}

	void	decodingChunk()
	{
		string decoded ="";
		string line;
		string::size_type start = 0;
		string::size_type end;
		long lineLength = 0;
		long totalLength = 0;

		while ( true )
		{
			end = content.find("\r\n", start);
			if (end == string::npos) throw exception();
			line = content.substr(start, end - start);
			if (line == "0") break;
			lineLength = strtol(line.c_str(), NULL, 16);
			if (lineLength == 0L) throw exception();
			totalLength += lineLength;
			start = start + (end - start) + 2;
			line = content.substr(start, lineLength);
			// lenght가 더 긴 경우 std::out_of_range발생
			decoded.append(line);
			start = start + lineLength + 2;

		}
		if ((long)decoded.length() != totalLength) throw exception();
		this->content = decoded;
	}

	void	clear() { content.clear(); /*IText::clear();*/ }
};
#endif
