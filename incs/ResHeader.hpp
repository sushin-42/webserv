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
	: IHeader(), statusCode(), reasonPhrase() {}
	ResHeader( const string& content )
	: IHeader(content), statusCode(), reasonPhrase() {}
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


	void	makeStatusLine();
	void	setDefaultHeaders();

	void	print();
};

IHeader::status_code_t	checkFile(const string& path);

#endif
