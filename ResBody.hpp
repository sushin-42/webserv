/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResBody.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mishin <mishin@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/09 19:22:25 by mishin            #+#    #+#             */
/*   Updated: 2022/05/11 15:54:05 by mishin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESBODY_HPP
# define RESBODY_HPP

#include "IText.hpp"
#include <iostream>
# include <string>
# include <sys/fcntl.h>
# include <unistd.h>
# include <fstream>
using namespace std;

class ResBody : public IText
{
public:

	ResBody(): IText() {}
	ResBody( const string& path ): IText()
	{
		readFile(path);
	}
	ResBody( const ResBody& src ): IText(src.content) {}
	~ResBody() {}

	ResBody&	operator=( const ResBody& src )
	{
		if (this != &src)
			content = src.content;
		return *this;
	}

	void	readFile( const string& path )
	{
		ifstream input_file(path);
	    if (!input_file.is_open())
		{
			cout << path << endl;
			exit(EXIT_FAILURE);	//FIXME
		}
    	content = string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
	}

	void	clear() { content.clear(); /*IText::clear();*/ }
};
#endif
