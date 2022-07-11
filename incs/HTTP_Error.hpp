#ifndef HTTPERROR_HPP
# define HTTPERROR_HPP

#include <exception>
#include <string>
using namespace std;

typedef unsigned short status_code_t;

class HTTP_Error: public exception
{
	private:	string msg;
	public:		status_code_t status;
				explicit HTTP_Error();
				explicit HTTP_Error(status_code_t s, const string& m);
				virtual ~HTTP_Error() throw();
				virtual const char * what() const throw();
};

class HTTP_3XX_Error: public HTTP_Error
{
	public:		string	location;
				explicit HTTP_3XX_Error();
				explicit HTTP_3XX_Error(status_code_t s, const string& m, const string& loc);
				virtual ~HTTP_3XX_Error() throw();
};

class HTTP_4XX_Error: public HTTP_Error
{
	public:		explicit HTTP_4XX_Error();
				explicit HTTP_4XX_Error(status_code_t s, const string& m);
				virtual ~HTTP_4XX_Error() throw();
};

class HTTP_5XX_Error: public HTTP_Error
{
	public:		explicit HTTP_5XX_Error();
				explicit HTTP_5XX_Error(status_code_t s, const string& m);
				virtual ~HTTP_5XX_Error() throw();
};









class badRequest: public HTTP_4XX_Error
{
	public:		explicit badRequest();
				explicit badRequest(status_code_t s, const string& m);
				virtual ~badRequest() throw();
};

class methodNotAllowed: public HTTP_4XX_Error
{
	public:		explicit methodNotAllowed();
				explicit methodNotAllowed(status_code_t s, const string& m);
				virtual ~methodNotAllowed() throw();
};

class notFound: public HTTP_4XX_Error
{
	public:		explicit notFound();
				explicit notFound(status_code_t s, const string& m);
				virtual ~notFound() throw();
};

class forbidden: public HTTP_4XX_Error
{
	public:		explicit forbidden();
				explicit forbidden(status_code_t s, const string& m);
				virtual ~forbidden() throw();
};

class internalServerError: public HTTP_5XX_Error
{
	public:		explicit internalServerError();
				explicit internalServerError(status_code_t s, const string& m);
				virtual ~internalServerError() throw();
};

class payloadTooLarge: public HTTP_4XX_Error
{
	public:		explicit payloadTooLarge();
				explicit payloadTooLarge(status_code_t s, const string& m);
				virtual ~payloadTooLarge() throw();
};

class lengthRequired: public HTTP_4XX_Error
{
	public:		explicit lengthRequired();
				explicit lengthRequired(status_code_t s, const string& m);
				virtual ~lengthRequired() throw();
};

class URITooLong: public HTTP_4XX_Error
{
	public:		explicit URITooLong();
				explicit URITooLong(status_code_t s, const string& m);
				virtual ~URITooLong() throw();
};


class Created: public HTTP_Error
{
	public:		string	location;
				explicit Created();
				explicit Created(status_code_t s, const string& m);
				virtual ~Created() throw();
};

class noContent: public HTTP_Error
{
	public:		string	location;
				explicit noContent();
				explicit noContent(const string& loc);
				virtual ~noContent() throw();
};

class Conflict: public HTTP_4XX_Error
{
	public:		explicit Conflict();
				explicit Conflict(status_code_t s, const string& m);
				virtual ~Conflict() throw();
};



class movedPermanently: public HTTP_3XX_Error
{
	public:		explicit movedPermanently();
				explicit movedPermanently(const string& loc);
				virtual ~movedPermanently() throw();
};

class found: public HTTP_3XX_Error
{
	public:		explicit found();
				explicit found(const string& loc);
				virtual ~found() throw();
};

class seeOther: public HTTP_3XX_Error
{
	public:		explicit seeOther();
				explicit seeOther(const string& loc);
				virtual ~seeOther() throw();
};
class temporaryRedirect: public HTTP_3XX_Error
{
	public:		explicit temporaryRedirect();
				explicit temporaryRedirect(const string& loc);
				virtual ~temporaryRedirect() throw();
};
class permanentRedirect: public HTTP_3XX_Error
{
	public:		explicit permanentRedirect();
				explicit permanentRedirect(const string& loc);
				virtual ~permanentRedirect() throw();
};

class notImplemented: public HTTP_5XX_Error
{
	public:		explicit notImplemented();
				explicit notImplemented(status_code_t s, const string& m);
				virtual ~notImplemented() throw();
};

class HTTPVersionNotSupported: public HTTP_5XX_Error
{
	public:		explicit HTTPVersionNotSupported();
				explicit HTTPVersionNotSupported(status_code_t s, const string& m);
				virtual ~HTTPVersionNotSupported() throw();
};

#endif
