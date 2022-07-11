#ifndef IHEADER_HPP
# define IHEADER_HPP
# include <map>
# include "IText.hpp"
# include "utils.hpp"

class IHeader: public IText
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
	IHeader() : IText() {}
	IHeader( const string& s ) : IText(s) {}
	IHeader( const IHeader& src )
	: IText(src.content), HTTPversion(src.HTTPversion), headerField(src.headerField) {}
	virtual ~IHeader() {}

/**========================================================================
* *                            operators
*========================================================================**/

	IHeader&	operator=( const IHeader& src )
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
