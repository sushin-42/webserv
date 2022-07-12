#ifndef UTILS_HPP
#define UTILS_HPP

#include <cctype>
#include <iostream>
#include <map>
#include <sstream>
#include <fstream>
#include <string>
#include <sys/_types/_ssize_t.h>
#include <unistd.h>
#include <pthread.h>
#include "color.hpp"

#define CONVERT(X, Y) dynamic_cast<Y *>(X)
#define CONVERT_REF(X, Y) dynamic_cast<Y &>(X)
#define WEBSERV_VERSION	"Webserv 0.1"
using namespace std;
class Config;
class PollSet;
typedef struct URI
{
	string	path;
	string	query;
	string	fragment;
} URI;

string fileToString(const string &path);

string getExt(const string &path);

inline std::string rtrim(std::string s, const char *t)
{
	s.erase(s.find_last_not_of(t) + 1);
	return s;
}

inline std::string ltrim(std::string s, const char *t)
{
	s.erase(0, s.find_first_not_of(t));
	return s;
}

std::string			trim(std::string s, const char *t);
string 				lowerize(const string &s);
map<string, string> KVtoMap(const string &content, char delim);
ssize_t				readFrom(int fd, string &content);
string&				replaceToken(string &content, const string &token, const string &value);
string 				capitalize(const string &s);
string 				capitalize(const string &s, char delim);
string 				errorpage(const string &title, const string &header, const string &message);
string				makeChunk(const string &s);
void				createServerSockets(map<
											pair<string, unsigned short>,
											vector<Config*>
										>& addrs );


/* inplace */
string 				extractHeader(string &content);
string				extractFirstWord(string& content);

string				pickOutRequestTarget(const string& content);
string				pickOutMethod(const string& content);
string 				pickOutHeader(const string &content, string lineTerminator);
string 				pickOutBody(const string &content);

pair<pid_t, int> whoDied();	/* we will check periodically all process forked. */

/*
	we don't allow obs-fold
	VCHAR  0x21~0x7E
*/
inline bool isVchar(int c) { return (0x21 <= c && c <= 0x7E); }
inline bool isOWS(int c) { return (c == ' ' || c == '\t'); }
inline bool isFieldchar(int c) { return (isVchar(c) || isOWS(c)); }

bool	isValidHeaderField(const string &line);
bool	isValidHeader(const string &content, string lineTerminator, bool parseStartLine);
bool	has2CRLF(const string &content);
bool	isNumber(const string &s);
bool 	argvError(int argc);
int		errMsg();

URI		splitRequestTarget(string reqTarget);



template <class T>
string toString(T whatever)
{
	stringstream out;
	out << whatever;
	return out.str();
}
template <class T>
string toHex(T num)
{
	stringstream sstream;
	sstream << std::hex << num;
	return sstream.str();
}

template <class T>
T	toHexNum(string s)
{
	T x;
	std::stringstream ss;
	ss << std::hex << s;
	ss >> x;

	return x;
}

template <class T>
T	toNum(const string& s)
{
	std::istringstream sstream(s);
	T num;
	sstream >> num;
	return num;
}

template <class _Key, class _Value>
const _Value& getValueIfExists(const map<_Key, _Value>& m, const _Key& key)
{
	typename map<_Key, _Value>::const_iterator it = m.find(key);

	if (it != m.end())
		return it->second;
	throw exception();
}



template <size_t>
struct sfinaer {};




template <class T>
struct hasLocation
{
// private:
    struct __two {char du; char mmy;};
	// static T	t;
    template <class U> static __two __test(...) {__two a; cout << "return TWO!" << endl; return a; };
    template <class U> static char  __test( sfinaer<sizeof(&U::location)>* ) { cout << "RETURN one!" << endl;return 'c'; };
public:
    static const bool value = sizeof(__test<T>(0)) == 1;
};
#endif
