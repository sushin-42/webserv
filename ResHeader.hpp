#ifndef RESHEADER_HPP
# define RESHEADER_HPP

#include "IHeader.hpp"
# include "ReqHeader.hpp"
#include <string>


class ReqHeader;
class ResHeader : public IHeader
{
private:
	status_code_t	statusCode;
	string			reasonPhrase;

public:
	ResHeader()
	: IHeader(), statusCode(), reasonPhrase() {}
	ResHeader( const string& content )
	: IHeader(content), statusCode(), reasonPhrase() {}
	ResHeader( const ResHeader& src )
	: IHeader(src), statusCode(src.statusCode), reasonPhrase(src.reasonPhrase) {}
	~ResHeader() {}

	ResHeader&	operator=( const ResHeader& src )
	{
		if (this != &src)
		{
			this->IHeader::operator=(src);
			this->statusCode	= src.statusCode;
			this->reasonPhrase	= src.reasonPhrase;
		}
		return *this;
	}


	void			setStatusCode(status_code_t s)		{ this->statusCode = s; }
	status_code_t	getStatusCode() const				{ return this->statusCode; }

	void			setReasonPhrase(const string& r)	{ this->reasonPhrase = r; }
	const string&	getReasonPhrase() const				{ return this->reasonPhrase; }


	void	makeStatusLine()
	{
		content.clear();
		content.append(
						HTTPversion + " " +
						toString(statusCode) + " " +
						reasonPhrase + "\r\n"
					);
	}

};

IHeader::status_code_t	checkFile(const string& path)
{
	int	requested	= open(path.c_str(), O_RDONLY);
	if (requested == -1)
	{
		if (errno == ENOENT) { return 404; }
	}
	close(requested);
	return 200;
}

#endif
