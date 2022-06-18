#ifndef ITEXT_HPP
# define ITEXT_HPP

# include <string>
using namespace std;

class IText
{
/**========================================================================
* %                          member variables
*========================================================================**/

protected:
	string	content;

/**========================================================================
* @                           Constructors
*========================================================================**/

public:
	IText() : content() {}
	IText( const string& s ) : content(s) {}
	IText( const IText& src ) : content(src.content) {}
	virtual ~IText() {}

/**========================================================================
* *                            operators
*========================================================================**/

	IText&	operator=( const IText& src )
	{
		if (this != &src)
			content = src.content;
		return *this;
	}

/**========================================================================
* #                          member functions
*========================================================================**/

	bool			empty() const;
	void			setContent(const string& c);
	string			getContent() const;
	void			append(const string& s);
	virtual void	clear() = 0;
};
#endif
