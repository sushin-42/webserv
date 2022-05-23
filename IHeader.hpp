/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IHeader.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mishin <mishin@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/09 18:26:49 by mishin            #+#    #+#             */
/*   Updated: 2022/05/24 00:17:41 by mishin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IHEADER_HPP
# define IHEADER_HPP
# include <map>
# include "IText.hpp"
# include "utils.hpp"

class IHeader: public IText
{
public:
	typedef unsigned short status_code_t;
protected:
	typedef	map<string, string>	_Map;
	typedef	_Map::iterator		iterator;

	string				HTTPversion;
	_Map				headerField;

public:
	IHeader() : IText() {}
	IHeader( const string& s ) : IText(s) {}
	IHeader( const IHeader& src ) : IText(src.content) {}
	virtual ~IHeader() {}

	IHeader&	operator=( const IHeader& src )
	{
		if (this != &src)
		{
			content		= src.content;
			headerField	= src.headerField;

		}
		return *this;
	}

	string& operator[](const string& key)
	{
		return headerField[lowerize(key)];
	}

	void						clear()											{ content.clear(); headerField.clear(); }
	void						setHTTPversion(const string& v)					{ this->HTTPversion = v; }
	const string&				getHTTPversion() const							{ return this->HTTPversion; }
	void						setHeaderField(const _Map& hf)	{ this->headerField = hf; }
	const map<string, string>&	getHeaderField() const							{ return this->headerField; }
	void						removeKey(const string& key)					{ this->headerField.erase(lowerize(key)); }
	void						insertRange(iterator from, iterator to)			{ this->headerField.insert(from, to); }

	void	integrate()
	{
		map<string, string>::iterator it;
		map<string, string>::iterator ite = headerField.end();
		for (it = headerField.begin(); it != ite; it++ )
			content.append(it->first + ": " + it->second + "\r\n");
		content.append("\r\n");
	}
};
#endif
