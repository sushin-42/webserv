#include "ResHeader.hpp"
#include "CGI.hpp"

/**========================================================================
* *                            operators
*========================================================================**/

ResHeader&	ResHeader::operator=( const ResHeader& src )
{
	if (this != &src)
	{
		this->IHeader::operator=(src);
		this->statusCode	= src.statusCode;
		this->reasonPhrase	= src.reasonPhrase;
	}
	return *this;
}

/**========================================================================
* #                          member functions
*========================================================================**/

void					ResHeader::setStatusCode(status_code_t s)		{ this->statusCode = s; }
IHeader::status_code_t	ResHeader::getStatusCode() const				{ return this->statusCode; }

void			ResHeader::setReasonPhrase(const string& r)	{ this->reasonPhrase = r; }
const string&	ResHeader::getReasonPhrase() const				{ return this->reasonPhrase; }


void	ResHeader::makeStatusLine()
{
	content.clear();
	content.append(
					HTTPversion + " " +
					toString(statusCode) + " " +
					reasonPhrase + "\r\n"
				);
}


void	ResHeader::setDefaultHeaders()
{
	(*this)["Connection"]			= "close";
	(*this)["Server"]				= "Webserv 0.1";
	// (*this)["Keep-Alive"]		=
	// (*this)["Last-Modified"]		=
	// (*this)["E-Tag"]				=
}

void	ResHeader::print()
{
	map<string, string>::iterator it = headerField.begin();
	map<string, string>::iterator ite = headerField.end();

	cout << "===========Header-Field===========" << endl;
	for (; it != ite; it++)
		cout << it->first << endl;
	cout << "==================================" << endl;
}

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

void	ResHeader::fetchStatusField()
{
	if (this->exist("status") == true)
	{
		pair<status_code_t, string>	sr = checkStatusField((*this)["Status"]);
		this->setStatusCode(sr.first);
		this->setReasonPhrase(sr.second);
		this->removeKey("Status");
	}
}
