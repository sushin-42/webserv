#include "httpError.hpp"
#include <iostream>

httpError::httpError(): msg(""), status(0) {}
httpError::httpError(status_code_t s, const string& m): msg(m), status(s) {}
httpError::~httpError() throw() {};
const char*	httpError::what() const throw() { return msg.c_str(); }

Created::Created(): httpError(201, "Created"), location() {}
Created::~Created() throw() {};

noContent::noContent(): httpError(204, "No Content"), location() {}
noContent::noContent(const string& loc): httpError(204, "No Content"), location(loc) {}
noContent::~noContent() throw() {};

redirectError::redirectError(): httpError(300, "Error"), location() {}
redirectError::redirectError(status_code_t s, const string& m, const string& loc): httpError(s, m), location(loc) {}
redirectError::~redirectError() throw() {};

movedPermanently::movedPermanently(): redirectError(301, "Moved Permanently", "") {}
movedPermanently::movedPermanently(const string& loc): redirectError(301, "Moved Permanently", loc) {}
movedPermanently::~movedPermanently() throw() {};

badRequest::badRequest(): httpError(400, "Bad Request") {}
badRequest::~badRequest() throw() {};

forbidden::forbidden(): httpError(403, "Forbidden") {}
forbidden::~forbidden() throw() {};

notFound::notFound(): httpError(404, "Not Found") {}
notFound::~notFound() throw() {};

methodNotAllowed::methodNotAllowed(): httpError(405, "Method Not Allowed") {}
methodNotAllowed::~methodNotAllowed() throw() {};

Conflict::Conflict(): httpError(409, "Conflict") {}
Conflict::~Conflict() throw() {};

lengthRequired::lengthRequired(): httpError(411, "Length Required") {}
lengthRequired::~lengthRequired() throw() {};

payloadTooLarge::payloadTooLarge(): httpError(413, "Payload Too Large") {}
payloadTooLarge::~payloadTooLarge() throw() {};

internalServerError::internalServerError(): httpError(500, "Internal Server Error") {}
internalServerError::~internalServerError() throw() {};






