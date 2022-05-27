/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IHeader.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mishin <mishin@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/09 18:26:49 by mishin            #+#    #+#             */
/*   Updated: 2022/05/27 11:39:28 by mishin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IHEADER_HPP
# define IHEADER_HPP
# include <map>
# include "IText.hpp"
# include "utils.hpp"

class IHeader: public IText
{
protected:
	typedef	map<string, string>	_Map;
public:
	typedef unsigned short		status_code_t;
	typedef	_Map::iterator		iterator;

	string				HTTPversion;
	_Map				headerField;

public:
	IHeader() : IText() {}
	IHeader( const string& s ) : IText(s) {}
	IHeader( const IHeader& src )
	: IText(src.content), HTTPversion(src.HTTPversion), headerField(src.headerField) {}
	virtual ~IHeader() {}

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

	void						clear()											{ content.clear(); headerField.clear(); }
	void						setHTTPversion(const string& v)					{ this->HTTPversion = v; }
	const string&				getHTTPversion() const							{ return this->HTTPversion; }
	void						setHeaderField(const _Map& hf)					{ this->headerField = hf; }
	const map<string, string>&	getHeaderField() const							{ return this->headerField; }
	void						removeKey(const string& key)					{ this->headerField.erase(lowerize(key)); }
	void						insertRange(iterator from, iterator to)			{ this->headerField.insert(from, to); }
	bool						exist(const string& key)						{ return this->headerField.find(lowerize(key)) != this->headerField.end(); }
	void						append(const string& key, const string& value)	{ this->headerField[lowerize(key)] += (", " + value); }

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
