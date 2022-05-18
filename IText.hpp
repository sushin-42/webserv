/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IText.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mishin <mishin@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/11 15:43:28 by mishin            #+#    #+#             */
/*   Updated: 2022/05/18 18:49:06 by mishin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ITEXT_HPP
# define ITEXT_HPP

# include <string>
using namespace std;

class IText
{
protected:
	string	content;
public:

	IText() : content() {}
	IText( const string& s ) : content(s) {}
	IText( const IText& src ) : content(src.content) {}
	virtual ~IText() {}

	IText&	operator=( const IText& src )
	{
		if (this != &src)
			content = src.content;
		return *this;
	}

	bool			empty() const		{ return this->content.empty(); }
	string			getContent() const	{ return this->content; }
	virtual void	clear() = 0;
};
#endif
