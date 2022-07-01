#ifndef EXCEPTIONS_HPP
# define EXCEPTIONS_HPP
# include <exception>
#include <string>

using namespace std;

class readMore: public exception
{
	public:		explicit readMore();
				virtual ~readMore() throw();
				virtual const char * what() const throw();
};

class sendMore: public exception
{
	public:		explicit sendMore();
				virtual ~sendMore() throw();
				virtual const char * what() const throw();
};

class gotoCore: public exception
{
public:		explicit gotoCore();
			virtual ~gotoCore() throw();
				virtual const char * what() const throw();
};

class internalRedirect: public exception
{
	public:		explicit internalRedirect();
				virtual ~internalRedirect() throw();
				virtual const char * what() const throw();
};

class sendMoreAfterRead: public exception
{
	public:		explicit sendMoreAfterRead();
				virtual ~sendMoreAfterRead() throw();
				virtual const char * what() const throw();
};

class autoIndex: public exception
{
	public:		string	 path;
				explicit autoIndex();
				explicit autoIndex(const string& p);
				virtual ~autoIndex() throw();
				virtual const char * what() const throw();
};

#endif
