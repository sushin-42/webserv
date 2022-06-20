#ifndef REQHEADER_HPP
# define REQHEADER_HPP
# include "IHeader.hpp"
# include "Config.hpp"
# include <fcntl.h>
#include <string>


class ReqHeader : public IHeader
{
/**========================================================================
* %                          member variables
*========================================================================**/

private:
	string	method;
	string	requestTarget;

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

	void			setRequsetTarget(const string& content);
	const string&	getRequsetTarget() const;
	void			setMethod(const string& m);
	const string&	getMethod() const;

	void		clear();
	void		print();
};
#endif
