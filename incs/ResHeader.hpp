#ifndef RESHEADER_HPP
# define RESHEADER_HPP

#include "AHeader.hpp"
# include "ReqHeader.hpp"
#include <string>

class ResHeader : public AHeader
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
	: AHeader(), statusCode(200), reasonPhrase("OK") {}
	ResHeader( const string& content )
	: AHeader(content), statusCode(200), reasonPhrase("OK") {}
	ResHeader( const ResHeader& src )
	: AHeader(src), statusCode(src.statusCode), reasonPhrase(src.reasonPhrase) {}
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

