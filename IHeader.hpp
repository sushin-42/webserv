/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IHeader.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mishin <mishin@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/09 18:26:49 by mishin            #+#    #+#             */
/*   Updated: 2022/05/21 13:39:40 by mishin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IHEADER_HPP
# define IHEADER_HPP
# include "IText.hpp"
#include <map>

class IHeader: public IText
{
public:
	typedef unsigned short status_code_t;
protected:
	string				HTTPversion;
	map<string, string>	headerField;

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
		return headerField[key];
	}

	void			clear()							{ content.clear(); headerField.clear(); }
	void			setHTTPversion(const string& v)	{ this->HTTPversion = v; }
	const string&	getHTTPversion() const			{ return this->HTTPversion; }

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
