#include "Exceptions.hpp"
#include <iostream>
httpError::httpError(): msg(""), status(500) {}
httpError::httpError(status_code_t s, const string& m): msg(m), status(s) {}
httpError::~httpError() throw() {};
const char*	httpError::what() const throw() { return msg.c_str(); }

badRequest::badRequest(): httpError(400, "Bad Request") {}
badRequest::~badRequest() throw() {};

methodNotAllowed::methodNotAllowed(): httpError(405, "Method Not Allowed") {}
methodNotAllowed::~methodNotAllowed() throw() {};
