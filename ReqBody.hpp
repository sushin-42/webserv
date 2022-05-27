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
private:
	long contentLength;
public:

	ReqBody(): IText(), contentLength(0) {}
	ReqBody( const ReqBody& src ): IText(src.content), contentLength(src.contentLength) {}
	~ReqBody() {}

	ReqBody&	operator=( const ReqBody& src )
	{
		if (this != &src)
		{
			content = src.content;
			contentLength = src.contentLength;
		}
		return *this;
	}

	void	setContentLength(long length) { this->contentLength = length; }
	long	getContentLength() const { return this->contentLength; }

	void	checkContentLength(string len)
	{
		long length = strtol(len.c_str(), NULL, 10);
		if (this->contentLength != length || length == 0L) throw exception();
	}

	void	decodingChunk()
	{
		string decoding ="";
		string line;
		string::size_type start = 0;
		string::size_type end;
		long length = 0;

		// while ((end = content.find("\r\n", start)) != string::npos)
		// {
		// 	line = content.substr(start, end - start);
		// 	if (line == "0") break;

		// 	length = strtol(line.c_str(), NULL, 16);

		// 	if (length == 0L) throw exception();
		// 	contentLength += length;
		// 	start = start + (end - start) + 2;

		// 	if ((end = content.find("\r\n", start)) != string::npos)
		// 		line = content.substr(start, end - start);
			
		// 	if ((long)line.length() != length) throw exception();
			
		// 	decoding.append(line);
		// 	start = start + length + 2;
		// }
		while ( true )
		{
			end = content.find("\r\n", start);
			line = content.substr(start, end - start);
			if (line == "0") break;

			length = strtol(line.c_str(), NULL, 16);
			if (length == 0L) throw exception();
			contentLength += length;
			start = start + (end - start) + 2;

			end = content.find("\r\n", start);
			line = content.substr(start, end - start);
			
			if ((long)line.length() != length) throw exception();
			
			decoding.append(line);
			start = start + length + 2;
		}
		if ((long)decoding.length() != this->contentLength) throw exception();
		this->content = decoding;
	}

	void	clear() { content.clear(); /*IText::clear();*/ }
};
#endif
