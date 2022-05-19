/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResHeader.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mishin <mishin@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/09 18:36:49 by mishin            #+#    #+#             */
/*   Updated: 2022/05/19 15:52:02 by mishin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESHEADER_HPP
# define RESHEADER_HPP

#include "IHeader.hpp"
# include "ReqHeader.hpp"


class ReqHeader;
class ResHeader : public IHeader
{
private:
	map<string, string>	table;

public:
	ResHeader(): IHeader(), table() {}
	ResHeader( const string& s): IHeader(s), table() {}
	ResHeader( const ResHeader& src ): IHeader(src), table(src.table) {}	//TODO
	~ResHeader() {}

	ResHeader&	operator=( const ResHeader& src )
	{
		if (this != &src)
		{
			this->content	= src.content;
			this->table		= src.table;
		}
		return *this;
	}

	void	replaceToken(const string& token, const string& value)
	{
		content.replace(content.find(token), token.length(), value);
	}

	string& operator[](const string& key)
	{
		return table[key];
	}

	void	makeStatusLine()
	{
		content.append(
						table["http-version"] + " " +
						table["status-code"] + " " +
						table["reason-phrase"] + "\r\n"
					);
	}
	void	integrate()
	{
		map<string, string>::iterator it;
		map<string, string>::iterator ite = table.end();
		for (it = table.begin(); it != ite; it++ )
		{
			if (it->first == "http-version"	||
				it->first == "status-code"	||
				it->first == "reason-phrase")
				continue;
			content.append(it->first + ": " + it->second + "\r\n");
		}
		content.append("\r\n");
	}
};

#endif
