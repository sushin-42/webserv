#include "ReqHeader.hpp"


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

	void	ReqHeader::setRequsetTarget(const string& content)
	{
		string::size_type start = content.find(" ") + 1;
		string::size_type end	= content.find(" ", start);

		this->requestTarget = content.substr(start, end - (start));
	}
	const string&	ReqHeader::getRequsetTarget() const	{ return this->requestTarget; }

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
