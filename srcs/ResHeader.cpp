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
	content.append(
					HTTPversion + " " +
					toString(statusCode) + " " +
					reasonPhrase + "\r\n"
				);
}

void	ResHeader::clearContent()
{
	content.clear();
}


void	ResHeader::setDefaultHeaders()
{
	(*this)["Server"]				= WEBSERV_VERSION;

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

void	ResHeader::clear()
{
	IHeader::clear();
	this->statusCode = 200;
	this->reasonPhrase = "OK";
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
