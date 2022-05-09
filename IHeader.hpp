/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IHeader.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mishin <mishin@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/09 18:26:49 by mishin            #+#    #+#             */
/*   Updated: 2022/05/09 20:55:29 by mishin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IHEADER_HPP
# define IHEADER_HPP

# include <string>
using namespace std;

// * example header
string headerTemplate = "#HTTP-VERSION #STATUS #REASON-PHARSE\n"
    					"Date: Fri, 01 Feb 2002 01:34:56 GMT\n"
    					"Server: webserv42\n"
    					"Connection: #CONNECTION\n"
    					"Content-Type: #MIME-TYPE\n\n";
class IHeader
{
protected:
	typedef unsigned short status_code_t;
	string	content;
public:
	IHeader() : content() {}
	IHeader( const string& s ) : content(s) {}
	IHeader( const IHeader& src ) : content(src.content) {}
	virtual ~IHeader() {}

	IHeader&	operator=( const IHeader& src )
	{
		if (this != &src)
			content = src.content;
		return *this;
	}

	string			getContent() const	{ return this->content; }
	virtual void	clear() = 0;
};
#endif
