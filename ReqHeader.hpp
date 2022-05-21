/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReqHeader.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mishin <mishin@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/09 18:30:30 by mishin            #+#    #+#             */
/*   Updated: 2022/05/21 13:40:18 by mishin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQHEADER_HPP
# define REQHEADER_HPP
# include "IHeader.hpp"
# include "Config.hpp"
# include <fcntl.h>
#include <string>

class ReqHeader : public IHeader
{
private:
	string	method;
	string	requestTarget;

public:
	ReqHeader()
	: IHeader(), method(), requestTarget() {}
	ReqHeader( const string& content )
	: IHeader(content), method(), requestTarget() {}
	ReqHeader( const ReqHeader& src )
	: IHeader(src), method(src.method), requestTarget(src.requestTarget) {}	//TODO
	~ReqHeader() {}

	ReqHeader&	operator=( const ReqHeader& src )
	{
		if (this != &src)
		{
			this->IHeader::operator=(src);
			this->method		= src.method;
			this->requestTarget	= src.requestTarget;
		}

		return *this;
	}

	void	setRequsetTarget()
	{
		string::size_type start = content.find(" ") + 1;
		string::size_type end	= content.find(" ", start);

		this->requestTarget = content.substr(start, end - (start));
	}
	const string&	getRequsetTarget() const	{ return this->requestTarget; }

	void			setMethod(const string& m)	{ this->method = m; }
	const string&	getMethod() const			{ return this->method; };

	void	clear()
	{
		IHeader::clear();
		method.clear();
		requestTarget.clear();
		HTTPversion.clear();
	}

};
#endif
