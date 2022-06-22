
# include "Exceptions.hpp"



readMore::readMore() {}
readMore::~readMore() throw() {}
const char * readMore::what() const throw() {return ""; }

sendMore::sendMore() {}
sendMore::~sendMore() throw() {}
const char * sendMore::what() const throw() {return ""; }


autoIndex::autoIndex() {}
autoIndex::autoIndex(const string& p): path(p) {}
autoIndex::~autoIndex() throw() {}
const char * autoIndex::what() const throw() {return ""; }




