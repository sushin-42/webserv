#ifndef HTTPERROR_HPP
# define HTTPERROR_HPP

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
	public:		explicit badRequest();
				explicit badRequest(status_code_t s, const string& m);
				virtual ~badRequest() throw();
};

class methodNotAllowed: public httpError
{
	public:		explicit methodNotAllowed();
				explicit methodNotAllowed(status_code_t s, const string& m);
				virtual ~methodNotAllowed() throw();
};

class notFound: public httpError
{
	public:		explicit notFound();
				explicit notFound(status_code_t s, const string& m);
				virtual ~notFound() throw();
};

class forbidden: public httpError
{
	public:		explicit forbidden();
				explicit forbidden(status_code_t s, const string& m);
				virtual ~forbidden() throw();
};

class internalServerError: public httpError
{
	public:		explicit internalServerError();
				explicit internalServerError(status_code_t s, const string& m);
				virtual ~internalServerError() throw();
};


#endif
