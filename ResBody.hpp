/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResBody.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mishin <mishin@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/09 19:22:25 by mishin            #+#    #+#             */
/*   Updated: 2022/05/09 21:19:44 by mishin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ResBody_HPP
# define ResBody_HPP

# include <string>
# include <sys/fcntl.h>
# include <unistd.h>
# include <fstream>
using namespace std;

class ResBody
{
private:
	string	content;

public:

	ResBody(): content() {}
	ResBody( const string& path ): content()
	{
		readFile(path);
	}
	ResBody( const ResBody& src ): content(src.content) {}
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
			exit(EXIT_FAILURE);	//FIXME
    	content = string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
	}
	string	getContent() const	{ return this->content; }
};
#endif
