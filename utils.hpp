/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mishin <mishin@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/06 15:03:17 by mishin            #+#    #+#             */
/*   Updated: 2022/05/06 17:49:55 by mishin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef UTILS_HPP
# define UTILS_HPP
#include <iostream>
#include <map>
# include <sstream>
# include <fstream>
#include <string>
using namespace std;

template <class T>
string toString(T whatever)
{
	stringstream out;
	out << whatever;
	return out.str();
}

string fileToString(const string& path)
{
	std::ifstream	ifs(path);
  	std::string		content( (std::istreambuf_iterator<char>(ifs) ),
                    		 (std::istreambuf_iterator<char>()    ) );

	return content;
}

map<string, string> getMIME()
{
	map<string, string> MIME;

	string	content = fileToString("./mime.types");
	string	type;
	string	ext;
	// string::size_type found = 0;
	string::size_type typestart = 0;
	string::size_type typeend = 0;
	string::size_type extstart = 0;
	string::size_type extend = 0;
	while (1)
	{
		typestart	= content.find_first_not_of(" \t\n;", extend);
		typeend		= content.find_first_of(" \t\n", typestart + 1);
		if (typestart == string::npos)
			break;

		type = content.substr(typestart, typeend-typestart);
		extend = content.find('|', typeend+1);
		while (content[extend] != ';')
		{
			extstart	= content.find_first_not_of(" \t", extend+1);
			extend		= content.find_first_of(" \t;", extstart+1);
			ext = content.substr(extstart, extend-extstart);
			MIME[ext] = type;
		}
	}
	return MIME;
}

string	getExt(const string& path)
{
	string ext = path.substr(path.rfind('.')+1, path.length()-1);
	return ext;
}
#endif
