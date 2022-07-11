#include "HTTP_Error.hpp"
#include <iostream>

HTTP_Error::HTTP_Error(): msg(""), status(0) {}
HTTP_Error::HTTP_Error(status_code_t s, const string& m): msg(m), status(s) {}
HTTP_Error::~HTTP_Error() throw() {};
const char*	HTTP_Error::what() const throw() { return msg.c_str(); }

HTTP_3XX_Error::HTTP_3XX_Error(): HTTP_Error(300, "Error"), location() {}
HTTP_3XX_Error::HTTP_3XX_Error(status_code_t s, const string& m, const string& loc): HTTP_Error(s, m), location(loc) {}
HTTP_3XX_Error::~HTTP_3XX_Error() throw() {};

HTTP_4XX_Error::HTTP_4XX_Error(): HTTP_Error(400, "Error") {}
HTTP_4XX_Error::HTTP_4XX_Error(status_code_t s, const string& m): HTTP_Error(s, m) {}
HTTP_4XX_Error::~HTTP_4XX_Error() throw() {};

HTTP_5XX_Error::HTTP_5XX_Error(): HTTP_Error(500, "Error") {}
HTTP_5XX_Error::HTTP_5XX_Error(status_code_t s, const string& m): HTTP_Error(s, m) {}
HTTP_5XX_Error::~HTTP_5XX_Error() throw() {};


Created::Created(): HTTP_Error(201, "Created"), location() {}
Created::~Created() throw() {};

noContent::noContent(): HTTP_Error(204, "No Content"), location() {}
noContent::noContent(const string& loc): HTTP_Error(204, "No Content"), location(loc) {}
noContent::~noContent() throw() {};


/*---------------------------------------------------------------------------*/



movedPermanently::movedPermanently(): HTTP_3XX_Error(301, "Moved Permanently", "") {}
movedPermanently::movedPermanently(const string& loc): HTTP_3XX_Error(301, "Moved Permanently", loc) {}
movedPermanently::~movedPermanently() throw() {};

found::found(): HTTP_3XX_Error(302, "Found", "") {}
found::found(const string& loc): HTTP_3XX_Error(302, "Found", loc) {}
found::~found() throw() {};

seeOther::seeOther(): HTTP_3XX_Error(303, "See Other", "") {}
seeOther::seeOther(const string& loc): HTTP_3XX_Error(303, "See Other", loc) {}
seeOther::~seeOther() throw() {};

temporaryRedirect::temporaryRedirect(): HTTP_3XX_Error(307, "Temporary Redirect", "") {}
temporaryRedirect::temporaryRedirect(const string& loc): HTTP_3XX_Error(307, "Temporary Redirect", loc) {}
temporaryRedirect::~temporaryRedirect() throw() {};

permanentRedirect::permanentRedirect(): HTTP_3XX_Error(308, "Permanent Redirect", "") {}
permanentRedirect::permanentRedirect(const string& loc): HTTP_3XX_Error(308, "Permanent Redirect", loc) {}
permanentRedirect::~permanentRedirect() throw() {};

/*---------------------------------------------------------------------------*/


badRequest::badRequest(): HTTP_4XX_Error(400, "Bad Request") {}
badRequest::~badRequest() throw() {};

forbidden::forbidden(): HTTP_4XX_Error(403, "Forbidden") {}
forbidden::~forbidden() throw() {};

notFound::notFound(): HTTP_4XX_Error(404, "Not Found") {}
notFound::~notFound() throw() {};

methodNotAllowed::methodNotAllowed(): HTTP_4XX_Error(405, "Method Not Allowed") {}
methodNotAllowed::~methodNotAllowed() throw() {};

Conflict::Conflict(): HTTP_4XX_Error(409, "Conflict") {}
Conflict::~Conflict() throw() {};

lengthRequired::lengthRequired(): HTTP_4XX_Error(411, "Length Required") {}
lengthRequired::~lengthRequired() throw() {};

payloadTooLarge::payloadTooLarge(): HTTP_4XX_Error(413, "Payload Too Large") {}
payloadTooLarge::~payloadTooLarge() throw() {};

URITooLong::URITooLong(): HTTP_4XX_Error(414, "URI Too Long") {}
URITooLong::~URITooLong() throw() {};


/*---------------------------------------------------------------------------*/


internalServerError::internalServerError(): HTTP_5XX_Error(500, "Internal Server Error") {}
internalServerError::~internalServerError() throw() {};

notImplemented::notImplemented(): HTTP_5XX_Error(501, "Not Implemented") {}
notImplemented::~notImplemented() throw() {};

HTTPVersionNotSupported::HTTPVersionNotSupported(): HTTP_5XX_Error(505, "HTTP Version Not Supported") {}
HTTPVersionNotSupported::~HTTPVersionNotSupported() throw() {};
