/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mishin <mishin@student.42seoul.kr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/06 15:03:17 by mishin            #+#    #+#             */
/*   Updated: 2022/05/23 08:27:05 by mishin           ###   ########.fr       */
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


inline std::string rtrim(std::string s, const char* t)
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

inline std::string ltrim(std::string s, const char* t)
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

inline std::string trim(std::string s, const char* t)
{
    return ltrim(rtrim(s, t), t);
}

map<string, string>	extractHeader(string content)	//NOTE: what if line endswith \r\n
{
	const char*			ws		= " \t\n\r\f\v";
	string::size_type	pStart	= 0;
	string::size_type	pEnd	= string::npos;
	string::size_type	pDelim	= 0;
	string				line;
	map<string, string>	ret;


	while ((pEnd = content.find('\n', pStart)) != string::npos)
	{
		line = content.substr(pStart, pEnd-pStart);
		if (line.empty() || line == "\r")	break;

		pDelim = line.find_first_of(":");
		if (pDelim == string::npos)	continue;
		ret[trim(line.substr(0,pDelim), ws)] = trim(line.substr(pDelim+1), ws);

		pStart = pEnd + 1;
	}
	return ret;
}

string lowerlize(const string& s)
{
	string				ret(s);
	string::iterator	it;
	string::iterator	ite = ret.end();

	for (it = ret.begin(); it < ite; it++)
		if (isupper(*it))
			*it = tolower(*it);
	return ret;
}
#endif
