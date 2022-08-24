#ifndef ITEXT_HPP
# define ITEXT_HPP

# include <string>
using namespace std;

class AText
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
	AText() : content() {}
	AText( const string& s ) : content(s) {}
	AText( const AText& src ) : content(src.content) {}
	virtual ~AText() {}

/**========================================================================
* *                            operators
*========================================================================**/

	AText&	operator=( const AText& src )
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
