#ifndef REQHEADER_HPP
# define REQHEADER_HPP
# include <fcntl.h>
# include <string>
# include "IHeader.hpp"
# include "Config.hpp"
# include "utils.hpp"


class ReqHeader : public IHeader
{
/**========================================================================
* %                          member variables
*========================================================================**/

private:
	string	method;
	string	requestTarget;
	URI		uri;

/**========================================================================
* @                           Constructors
*========================================================================**/

public:
	ReqHeader()
	: IHeader(), method(), requestTarget() {}
	ReqHeader( const string& content )
	: IHeader(content), method(), requestTarget() {}
	ReqHeader( const ReqHeader& src )
	: IHeader(src), method(src.method), requestTarget(src.requestTarget) {}	//TODO
	~ReqHeader() {}

/**========================================================================
* *                            operators
*========================================================================**/

	ReqHeader&	operator=( const ReqHeader& src );

/**========================================================================
* #                          member functions
*========================================================================**/

	void			setHTTPversion(const string& v);
	void			setRequestTarget(const string& content);
	const string&	getRequestTarget() const;
	void			setURI(const URI& uri);
	const URI&		getURI() const;
	void			setMethod(const string& m);
	const string&	getMethod() const;

	void		clear();
	void		print();
};
#endif
