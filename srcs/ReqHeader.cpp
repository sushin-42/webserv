#include "ReqHeader.hpp"
#include "HTTP_Error.hpp"


/**========================================================================
* *                            operators
*========================================================================**/

	ReqHeader&	ReqHeader::operator=( const ReqHeader& src )
	{
		if (this != &src)
		{
			this->IHeader::operator=(src);
			this->method		= src.method;
			this->requestTarget	= src.requestTarget;
		}

		return *this;
	}

/**========================================================================
* #                          member functions
*========================================================================**/

	void			ReqHeader::setURI(const URI& uri)	{ this->uri = uri; }
	const URI&		ReqHeader::getURI() const			{ return this->uri; }

	void			ReqHeader::setRequestTarget(const string& s){
																	if (s.length() > 8192) /* 8KB */
																		throw URITooLong();
																	this->requestTarget = s;
																}
	const string&	ReqHeader::getRequestTarget() const	{ return this->requestTarget; }

	void			ReqHeader::setMethod(const string& m)	{ this->method = m; }
	const string&	ReqHeader::getMethod() const			{ return this->method; };

	void	ReqHeader::clear()
	{
		IHeader::clear();
		method.clear();
		requestTarget.clear();
		HTTPversion.clear();
	}

	void	ReqHeader::print()
	{
		map<string, string>::iterator it = headerField.begin();
		map<string, string>::iterator ite = headerField.end();

		cout << "===========Header-Field===========" << endl;
		for (; it != ite; it++)
			cout << it->first << endl;
		cout << "==================================" << endl;
	}

	void	ReqHeader::setHTTPversion(const string &v)
	{
		string	protocol = "HTTP/";
		string	version = v.substr(protocol.length());

		if (v.length() < 8)					throw badRequest();
		if (v.find(protocol) != 0)			throw badRequest();

		string::size_type pos = 0;
		if (isdigit(version[pos]) == false)	throw badRequest();
		for	(; pos < version.length(); pos++)
			if (isdigit(version[pos]) == false)
				break;

		if (version[pos++] != '.')					throw badRequest();
		if (pos >= version.length())				throw badRequest();

		for	(; pos < version.length(); pos++)
			if (isdigit(version[pos]) == false)
				break;
		if (pos != version.length())				throw badRequest();

		char*	t;
		double	ver;
		ver = strtod(version.c_str(), &t);
		if (ver != 1.1)								throw HTTPVersionNotSupported();

		IHeader::setHTTPversion(v);
	}
