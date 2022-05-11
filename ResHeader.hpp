/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResHeader.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mishin <mishin@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/09 18:36:49 by mishin            #+#    #+#             */
/*   Updated: 2022/05/11 16:29:03 by mishin           ###   ########.fr       */
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

public:
	ResHeader(): IHeader() {}
	ResHeader( const string& s): IHeader(s) {}
	ResHeader( const ResHeader& src ): IHeader(src) {}	//TODO
	~ResHeader() {}

	ResHeader&	operator=( const ResHeader& src )
	{
		if (this != &src)
			this->content = src.content;
		return *this;
	}

	void	replaceToken(const string& token, const string& value)
	{
		content.replace(content.find(token), token.length(), value);
	}
};

ResHeader	makeResponseHeader(const ReqHeader& req)
{
	ResHeader	res(headerTemplate);
	res.replaceToken("#HTTP-VERSION", "HTTP/1.1");
	res.replaceToken("#STATUS", toString(req.getStatus()));
	res.replaceToken("#REASON-PHARSE", req.reason);
	res.replaceToken("#MIME-TYPE", MIME[getExt(req.path)]);
	res.replaceToken("#CONNECTION", "keep-alive");
	return res;
}

#endif
