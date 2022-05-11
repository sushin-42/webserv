/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IHeader.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mishin <mishin@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/09 18:26:49 by mishin            #+#    #+#             */
/*   Updated: 2022/05/11 16:04:01 by mishin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IHEADER_HPP
# define IHEADER_HPP
# include "IText.hpp"

// * example header
string headerTemplate = "#HTTP-VERSION #STATUS #REASON-PHARSE\n"
    					"Date: Fri, 01 Feb 2002 01:34:56 GMT\n"
    					"Server: webserv42\n"
    					"Connection: #CONNECTION\n"
    					"Content-Type: #MIME-TYPE\n\n";
class IHeader: public IText
{
public:
	typedef unsigned short status_code_t;
protected:
	status_code_t	status;

public:
	IHeader() : IText() {}
	IHeader( const string& s ) : IText(s) {}
	IHeader( const IHeader& src ) : IText(src.content) {}
	virtual ~IHeader() {}

	IHeader&	operator=( const IHeader& src )
	{
		if (this != &src)
			content = src.content;
		return *this;
	}

	void			clear()				{ content.clear(); status = -1; }
	status_code_t	getStatus()	const	{ return status; }
};
#endif
