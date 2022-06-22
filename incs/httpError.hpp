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

class payloadTooLarge: public httpError
{
	public:		explicit payloadTooLarge();
				explicit payloadTooLarge(status_code_t s, const string& m);
				virtual ~payloadTooLarge() throw();
};

class lengthRequired: public httpError
{
	public:		explicit lengthRequired();
				explicit lengthRequired(status_code_t s, const string& m);
				virtual ~lengthRequired() throw();
};

class Created: public httpError
{
	public:		string	location;
				explicit Created();
				explicit Created(status_code_t s, const string& m);
				virtual ~Created() throw();
};

class noContent: public httpError
{
	public:		string	location;
				explicit noContent();
				explicit noContent(const string& loc);
				virtual ~noContent() throw();
};

class Conflict: public httpError
{
	public:		string	location;
				explicit Conflict();
				explicit Conflict(status_code_t s, const string& m);
				virtual ~Conflict() throw();
};

class redirectError: public httpError
{
	public:		string	location;
				explicit redirectError();
				explicit redirectError(status_code_t s, const string& m, const string& loc);
				virtual ~redirectError() throw();
};

class movedPermanently: public redirectError
{
	public:		explicit movedPermanently();
				explicit movedPermanently(const string& loc);
				virtual ~movedPermanently() throw();
};

#endif
