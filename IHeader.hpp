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
		return headerField.at(lowerize(key));	// MAY throw exception if key not exists.
	}

/**========================================================================
* #                          member functions
*========================================================================**/

	void						clear()											{ content.clear(); headerField.clear(); }
	void						setHTTPversion(const string& v)					{ this->HTTPversion = v; }
	const string&				getHTTPversion() const							{ return this->HTTPversion; }
	void						setHeaderField(const _Map& hf)					{ this->headerField = hf; }
	const map<string, string>&	getHeaderField() const							{ return this->headerField; }
	void						removeKey(const string& key)					{ this->headerField.erase(lowerize(key)); }
	void						insertRange(iterator from, iterator to)			{ this->headerField.insert(from, to); }
	bool						exist(const string& key)						{ return this->headerField.find(lowerize(key)) != this->headerField.end(); }

	void	append(const string& key, const string& value)
	{
		if (this->exist(key))
		{
			if ((*this)[key] != value)
				(*this)[key] += (", " + value);
		}
		else
			(*this)[key] = value;
	}

	void	integrate()
	{
		map<string, string>::iterator it;
		map<string, string>::iterator ite = headerField.end();
		for (it = headerField.begin(); it != ite; it++ )
			content.append(capitalize(it->first, '-') + ": " + it->second + "\r\n");
		content.append("\r\n");
	}
};
#endif
