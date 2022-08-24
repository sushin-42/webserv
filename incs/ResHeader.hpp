#ifndef RESHEADER_HPP
# define RESHEADER_HPP

#include "IHeader.hpp"
# include "ReqHeader.hpp"
#include <string>

class ResHeader : public IHeader
{
/**========================================================================
* %                          member variables
*========================================================================**/

private:
	status_code_t	statusCode;
	string			reasonPhrase;

/**========================================================================
* @                           Constructors
*========================================================================**/

public:
	ResHeader()
	: IHeader(), statusCode(200), reasonPhrase("OK") {}
	ResHeader( const string& content )
	: IHeader(content), statusCode(200), reasonPhrase("OK") {}
	ResHeader( const ResHeader& src )
	: IHeader(src), statusCode(src.statusCode), reasonPhrase(src.reasonPhrase) {}
	~ResHeader() {}

/**========================================================================
* *                            operators
*========================================================================**/

	ResHeader&	operator=( const ResHeader& src );

/**========================================================================
* #                          member functions
*========================================================================**/

	void			setStatusCode(status_code_t s);
	status_code_t	getStatusCode() const;
	void			setReasonPhrase(const string& r);
	const string&	getReasonPhrase() const;
	void			clearContent();


	void	makeStatusLine();
	void	setDefaultHeaders();
	void	fetchStatusField();

	void	print();
	void	clear();
};

#endif

