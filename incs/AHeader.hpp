#ifndef IHEADER_HPP
# define IHEADER_HPP
# include <map>
# include "AText.hpp"
# include "utils.hpp"

class AHeader: public AText
{
/**========================================================================
* '                              typedefs
*========================================================================**/

protected:
	typedef	map<string, string>	_Map;
public:
	typedef unsigned short		status_code_t;
	typedef	_Map::iterator		iterator;

/**========================================================================
* %                          member variables
*========================================================================**/

	string				HTTPversion;
	_Map				headerField;

/**========================================================================
* @                           Constructors
*========================================================================**/

public:
	AHeader() : AText() {}
	AHeader( const string& s ) : AText(s) {}
	AHeader( const AHeader& src )
	: AText(src.content), HTTPversion(src.HTTPversion), headerField(src.headerField) {}
	virtual ~AHeader() {}

/**========================================================================
* *                            operators
*========================================================================**/

	AHeader&	operator=( const AHeader& src )
	{
		if (this != &src)
		{
			content		= src.content;
			HTTPversion	= src.HTTPversion;
			headerField	= src.headerField;
		}
		return *this;
	}

	string& operator[](const string& key)
	{
		return headerField[lowerize(key)];
	}

	const string& operator[](const string& key) const
	{
		return getValueIfExists(headerField, lowerize(key));
	}

/**========================================================================
* #                          member functions
*========================================================================**/

	void						clear();
	void						setHTTPversion(const string& v);
	const string&				getHTTPversion() const;
	void						setHeaderField(const _Map& hf);
	const map<string, string>&	getHeaderField() const;
	void						removeKey(const string& key);
	void						insertRange(iterator from, iterator to);
	bool						exist(const string& key);

	void						append(const string& key, const string& value);
	void						integrate();
};
#endif
