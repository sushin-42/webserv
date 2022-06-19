#ifndef EXCEPTIONS_HPP
# define EXCEPTIONS_HPP

#include <exception>
#include <string>
using namespace std;

typedef unsigned short status_code_t;

class httpError: public exception
{
	private:	string msg;
	public:		status_code_t status;
				explicit httpError();
				explicit httpError(status_code_t s, const string& m);
				virtual ~httpError() throw();
				virtual const char * what() const throw();
};

class badRequest: public httpError
{
	private:	string msg;
	public:		explicit badRequest();
				explicit badRequest(status_code_t s, const string& m);
				virtual ~badRequest() throw();
};

class methodNotAllowed: public httpError
{
	private:	string msg;
	public:		explicit methodNotAllowed();
				explicit methodNotAllowed(status_code_t s, const string& m);
				virtual ~methodNotAllowed() throw();
};


#endif
