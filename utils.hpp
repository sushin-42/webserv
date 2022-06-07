#ifndef UTILS_HPP
# define UTILS_HPP

#include <cctype>
#include <iostream>
#include <map>
# include <sstream>
# include <fstream>
#include <string>
#include <sys/_types/_ssize_t.h>
#include <unistd.h>
#include "color.hpp"
#define  CONVERT(X, Y) dynamic_cast<Y*>(X)
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

ssize_t	readFrom(int fd, string& content)
{
	ssize_t byte = 0;
	char readbuf[1024];
	bzero(readbuf, sizeof(readbuf));
	while ((byte = read(fd, readbuf, sizeof(readbuf))) > 0)
	{
		TAG(utils, readFrom); cout << GRAY ("read ") << _UL << byte << _NC << GRAY(" bytes from ") << _UL << fd  << _NC << endl;
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

template <class T>
string	toHex(T num)
{
	stringstream sstream;
	sstream << std::hex << num;
	return sstream.str();

}

template <class T>
T	toNum(const string& s)
{
	std::istringstream sstream(s);
	T num;
	sstream >> num;
	return num;
}

string	makeChunk(const string& s)
{
	return (toHex(s.length()) + "\n" + s + "\n");
}


/* we will check periodically all process forked. */
pair<pid_t, int> whoDied()
{
	int	status = 0;

	return make_pair(waitpid(-1, &status, WNOHANG), status);
}


/*
	we don't allow obs-fold
	VCHAR  0x21~0x7E
*/

inline bool	isVchar(int c)		{ return (0x21 <= c && c <=0x7E); }
inline bool	isOWS(int c)		{ return (c == ' ' || c == '\t'); }
inline bool	isFieldchar(int c)	{ return (isVchar(c) || isOWS(c)); }

bool	isValidHeaderField(const string& line)
{
	string::size_type	pDelim	= 0;
	string				name;
	string				value;

	if (find_if_not(line.begin(),line.end(),isFieldchar) != line.end())
		return false;

	pDelim = line.find_first_of(':');
	if (pDelim == string::npos)
		return false;

	name  = line.substr(0,pDelim);
	value = line.substr(pDelim+1);

	if (name.find_first_not_of(" \t") != 0)
		return false;

	if (isspace(name.back()))
		return false;

	return true;
}

bool	isValidHeader(const string& content)
{
	string::size_type	pStart	= 0;
	string::size_type	pEnd	= string::npos;
	string				line;

	if (content.empty())	return false;

	pEnd = content.find("\r\n", pStart);	// parse start-line
	pStart = pEnd + 2;

	while ((pEnd = content.find("\r\n", pStart)) != string::npos)
	{
		line = content.substr(pStart, pEnd-pStart);
		if (line.empty())	break;
		if (!isValidHeaderField(line))	return false;
		pStart = pEnd + 2;
	}
	return true;
}

bool	has2CRLF(const string& content)
{
	return (content.find("\r\n\r\n") != string::npos);
}

bool	isNumber(const string& s)
{

    string::const_iterator it = s.begin();
    while (it != s.end() && isdigit(*it)) ++it;

	return (!s.empty() && it == s.end());
}


#endif

