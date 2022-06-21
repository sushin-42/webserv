#ifndef EXCEPTIONS_HPP
# define EXCEPTIONS_HPP
# include <exception>

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

class autoIndex: public exception
{
	public:		explicit autoIndex();
				virtual ~autoIndex() throw();
				virtual const char * what() const throw();
};

#endif
