#ifndef UTILS_HPP
# define UTILS_HPP
#include <cctype>
#include <iostream>
#include <map>
# include <sstream>
# include <fstream>
#include <string>
#include <unistd.h>
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

string lowerize(const string& s)
{
	string				ret(s);
	string::iterator	it;
	string::iterator	ite = ret.end();

	for (it = ret.begin(); it < ite; it++)
		if (isupper(*it))
			*it = tolower(*it);
	return ret;
}

map<string, string>	KVtoMap(const string& content, char delim)
{
	const char*			ws		= " \t\r\f\v";
	string::size_type	pStart	= 0;
	string::size_type	pEnd	= string::npos;
	string::size_type	pDelim	= 0;
	string				line;
	map<string, string>	ret;

	while ((pEnd = content.find('\n', pStart)) != string::npos)
	{
		line = trim(content.substr(pStart, pEnd-pStart), "\r");
		if (line.empty())	break;
		pStart = pEnd + 1;

		pDelim = line.find_first_of(delim);
		if (pDelim == string::npos)	continue;
		ret[lowerize(trim(line.substr(0,pDelim), ws))] = trim(line.substr(pDelim+1), ws);
	}
	return ret;
}

string	extractHeader(const string& content)
{
	string::size_type	pStart	= 0;
	string::size_type	pEnd	= string::npos;

	while ((pEnd = content.find('\n', pStart)) != string::npos)
	{
		if	((content[pEnd + 1] == '\n') ||
			 (content[pEnd + 1] == '\r' && content[pEnd + 2] == '\n'))
			break;
		pStart = pEnd + 1;
	}
	return (content.substr(0, pEnd));
}

string	extractBody(const string& content)
{
	string::size_type	pStart	= 0;
	string::size_type	pEnd	= string::npos;
	string::size_type	offset	= 0;

	while ((pEnd = content.find('\n', pStart)) != string::npos)
	{
		if		(content[pEnd + 1] == '\n')	{offset = 1; break;}
		else if	(content[pEnd + 1] == '\r' &&
				 content[pEnd + 2] == '\n')	{offset = 2; break;}
		pStart = pEnd + 1;
	}
	return (content.substr(pEnd + offset + 1));
}

ssize_t				readFrom(int fd, string& content)
{
	ssize_t byte = 0;
	char readbuf[1024];
	bzero(readbuf, sizeof(readbuf));
	while ((byte = read(fd, readbuf, sizeof(readbuf))) >0)
	{
		// cout << "keep reading.." << endl;
		// cout << "byte = " << byte << endl;
		content.append(readbuf, byte);
		bzero(readbuf, sizeof(readbuf));
	}
	return byte;
}

string&	replaceToken(string& content, const string& token, const string& value)
{
	content.replace(content.find(token), token.length(), value);
	return content;
}


string	capitalize(const string& s)
{
	string ret = lowerize(s);
	char   offset = 'a' - 'A';

	if (ret[0] && isalpha(ret[0]))
		ret[0] -= offset;

	return ret;
}

string	capitalize(const string& s, char delim)
{
	string::size_type	pStart	= 0;
	string::size_type	pDelim	= 0;

	char	offset = 'a' - 'A';
	string	ret	= capitalize(s);
	while (1)
	{
		pDelim = ret.find(delim, pStart);

		if (pDelim == string::npos)
			break;
		else
		{
			if (islower(ret[pDelim + 1]))
				ret[pDelim + 1] -= offset;
		}

		pStart = pDelim + 1;
	}
	return ret;
}

string	errorpage(const string& title, const string& header, const string& message)
{
	static string tmpl= "<!DOCTYPE HTML>\n"
						"<html>\n"
							"<head>\n"
								"<title>#TITLE</title>\n"
							"</head>\n"
							"<body>\n"
								"<h1>#HEADER</h1>\n"
								"<p>#MESSAGE</p>\n"
							"</body>\n"
						"</html>\n";
	string ret(tmpl);
	replaceToken(
		replaceToken(
			replaceToken(ret,
				 "#TITLE", title),
		 "#HEADER", header),
	"#MESSAGE", message);

	return ret;
	//The document has moved <a href=\"http://localhost:8080/es.png\">here</a>.
}
#endif
