#include "utils.hpp"
#include "ServerSocket.hpp"
#include "Poll.hpp"
#include "ServerConfig.hpp"
#include <string>


string fileToString(const string &path)
{
	std::ifstream ifs(path);
	std::string content((std::istreambuf_iterator<char>(ifs)),
						(std::istreambuf_iterator<char>()));

	return content;
}

string getExt(const string &path)
{
	string ext = path.substr(path.rfind('.') + 1, path.length() - 1);
	return ext;
}

std::string trim(std::string s, const char *t)
{
	return ltrim(rtrim(s, t), t);
}

string lowerize(const string &s)
{
	string ret(s);
	string::iterator it;
	string::iterator ite = ret.end();

	for (it = ret.begin(); it < ite; it++)
		if (isupper(*it))
			*it = tolower(*it);
	return ret;
}

map<string, string> KVtoMap(const string &content, char delim)
{
	const char *ws = " \t\r\f\v";
	string::size_type pStart = 0;
	string::size_type pEnd = string::npos;
	string::size_type pDelim = 0;
	string line;
	map<string, string> ret;

	while ((pEnd = content.find('\n', pStart)) != string::npos)
	{
		line = trim(content.substr(pStart, pEnd - pStart), "\r");
		if (line.empty())
			break;
		pStart = pEnd + 1;

		pDelim = line.find_first_of(delim);
		if (pDelim == string::npos)
			continue;
		ret[lowerize(trim(line.substr(0, pDelim), ws))] = trim(line.substr(pDelim + 1), ws);
	}
	return ret;
}

string extractHeader(string& content)
{
	string 				header;
	string::size_type	pStart = 0;
	string::size_type	pEnd = string::npos;
	string::size_type	offset;

	while ((pEnd = content.find('\n', pStart)) != string::npos)
	{
		if ((content[pEnd + 1] == '\n' && (offset = 2)) ||
			(content[pEnd + 1] == '\r' && content[pEnd + 2] == '\n' && (offset = 3)))
			break;
		pStart = pEnd + 1;
	}
	header = content.substr(0, pEnd + offset);
	content = content.substr(pEnd + offset);
	return (header);
}

string pickOutHeader(const string &content, string lineTerminator)
{
	string::size_type offset = lineTerminator.length() * 2;
	string::size_type pos =	content.find(lineTerminator+lineTerminator);

	if (pos == string::npos)
		return "";
	return content.substr(0, pos + offset);
}

string pickOutBody(const string &content)
{
	string::size_type pStart = 0;
	string::size_type pEnd = string::npos;
	string::size_type offset = 0;

	while ((pEnd = content.find('\n', pStart)) != string::npos)
	{
		if (content[pEnd + 1] == '\n')
		{
			offset = 1;
			break;
		}
		else if (content[pEnd + 1] == '\r' &&
				 content[pEnd + 2] == '\n')
		{
			offset = 2;
			break;
		}
		pStart = pEnd + 1;
	}
	if (pEnd == string::npos)
		return "";
	return (content.substr(pEnd + offset + 1));
}

string pickOutMethod(const string& content)
{
	string::size_type end = content.find(" ");
	string method = content.substr(0, end);
	return method;
}

ssize_t readFrom(int fd, string &content)
{
	ssize_t byte = 0;
	char readbuf[32768];
	bzero(readbuf, sizeof(readbuf));
	byte = read(fd, readbuf, sizeof(readbuf));
	switch (byte)
	{
	case 0:
		LOGGING(utils, GRAY ("read ") UL("%zu") " bytes from " UL("%d"), byte, fd);
		break;
	case -1:
		LOGGING(utils, GRAY ("read ") UL("%zu") " bytes from " UL("%d"), byte, fd);
		LOGGING(utils, RED("Error : ") "%s", strerror(errno));
		break;
	default:
		LOGGING(utils, GRAY ("read ") UL("%zu") " bytes from " UL("%d"), byte, fd);
		content.append(readbuf, byte);
	}
	return byte;
}

string &replaceToken(string &content, const string &token, const string &value)
{
	content.replace(content.find(token), token.length(), value);
	return content;
}

string capitalize(const string &s)
{
	string ret = lowerize(s);
	char offset = 'a' - 'A';

	if (ret[0] && isalpha(ret[0]))
		ret[0] -= offset;

	return ret;
}

string capitalize(const string &s, char delim)
{
	string::size_type pStart = 0;
	string::size_type pDelim = 0;

	char offset = 'a' - 'A';
	string ret = capitalize(s);
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

string errorpage(const string &title, const string &header, const string &message)
{
	static string tmpl = "<!DOCTYPE HTML>\n"
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
	// The document has moved <a href=\"http://localhost:8080/es.png\">here</a>.
}

string makeChunk(const string &s)
{
	return (toHex(s.length()) + "\r\n" + s + "\r\n");
}

/* we will check periodically all process forked. */
pair<pid_t, int> whoDied()
{
	int status = 0;

	return make_pair(waitpid(-1, &status, WNOHANG), status);
}
/*
	we don't allow obs-fold
	VCHAR  0x21~0x7E
*/
bool isValidHeaderField(const string &line)
{
	string::size_type pDelim = 0;
	string name;
	string value;

	if (find_if_not(line.begin(), line.end(), isFieldchar) != line.end())
		return false;

	pDelim = line.find_first_of(':');
	if (pDelim == string::npos)
		return false;

	name = line.substr(0, pDelim);
	value = line.substr(pDelim + 1);

	if (name.find_first_not_of(" \t") != 0)
		return false;

	if (isspace(name[name.length() - 1]))
		return false;

	return true;
}

bool isValidHeader(const string &content, string lineTerminator, bool parseStartLine)
{
	string::size_type pStart = 0;
	string::size_type pEnd = string::npos;
	string line;

	if (content.empty())
		return false;

	pEnd = content.find(lineTerminator, pStart);

	if (parseStartLine)
	{
		/* parse start-line	 */
		line = content.substr(pStart, pEnd - pStart);

		string::size_type posSP	= 0;
		int					sp	= 0;
		for (; posSP < pEnd; posSP++)
		{
			if (line[posSP] == ' ')
			{
				if ( posSP == 0)	return false;
				if ( line.find_first_not_of("\t\r\n\f\v ", posSP) != posSP + 1 ) return false;
				else sp++;
			}
		}
		if (sp != 2) return false;
		pStart = pEnd + lineTerminator.length();
	}

	/* parse header field */

	while ((pEnd = content.find(lineTerminator, pStart)) != string::npos)
	{
		line = content.substr(pStart, pEnd - pStart);
		if (line.empty())
			break;
		if (!isValidHeaderField(line))
			return false;
		pStart = pEnd + lineTerminator.length();
	}
	return true;
}


bool has2CRLF(const string &content)
{
	return (content.find("\r\n\r\n") != string::npos);
}

bool isNumber(const string &s)
{

	string::const_iterator it = s.begin();
	while (it != s.end() && isdigit(*it))
		++it;

	return (!s.empty() && it == s.end());
}

bool argvError(int argc)
{
	return (argc > 2);
}

int errMsg()
{
	cerr << RED("argument error") << endl;
	return (-1);
}


void				createServerSockets(map<
											pair<string, unsigned short>,
											vector<Config*>
										>& addrs )
{
	ServerSocket* serv;
	map<
		pair<string, unsigned short>,
		vector<Config*>
	>::iterator mit, mite;
	mit = addrs.begin(), mite = addrs.end();



	for (;mit != mite; mit++)
	{
		serv = new ServerSocket(mit->first.first, mit->first.second);
		int optval = 1;
		setsockopt(serv->getFD(), SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

		cout << "-------------------------" << endl;
		try						{ serv->bind(); }
		catch (exception& e)	{ cerr << e.what() << endl; exit(errno); }
		try						{ serv->listen(SOMAXCONN /*backlog*/); }
		catch (exception& e)	{ cerr << e.what() << endl; exit(errno); }
		serv->confs = mit->second;
		vector<Config*>::iterator confIT = serv->confs.begin(); 
		for (; confIT < serv->confs.end(); confIT++) 
		{
			vector<string>::iterator it, ite;
			it = CONVERT(*confIT, ServerConfig)->server_names.begin();
			ite = CONVERT(*confIT, ServerConfig)->server_names.end();
			for (; it < ite; it++)
				cout << "  •  " << (it->empty() ? "(empty name)" : *it) << endl;
		}
		cout << "-------------------------" << endl << endl;

		POLLSET->enroll(serv, POLLIN);
	}


}

string	extractFirstWord(string& content)
{
	string::size_type offset;
	string::size_type posSP = content.find_first_of(" \r");
	string word = content.substr(0, posSP);

	if		(content[posSP] == ' ')	offset = 1;
	else if	(content[posSP] == '\r')offset = 2;
	else	{cout << "HERE" << endl; throw badRequest();}	// noreach

	content = content.substr(posSP + offset);
	return word;
}

string	pickOutRequestTarget(const string& content)
{
	string::size_type start = content.find(" ") + 1;
	string::size_type end	= content.find(" ", start);

	return content.substr(start, end - (start));
}

URI		splitRequestTarget(string reqTarget)
{
	URI	ret = (URI){reqTarget, "", ""};

	string::size_type	pathEnd, queryEnd;

	pathEnd = reqTarget.find_first_of("?#");
	if (pathEnd == string::npos)	return ret;
	else
	{
		ret.path = reqTarget.substr(0,pathEnd);

		if (reqTarget[pathEnd] == '?')
		{
			queryEnd = reqTarget.find('#', pathEnd + 1);
			ret.query = reqTarget.substr(
										pathEnd + 1,
										queryEnd == string::npos ?
											string::npos : queryEnd - (pathEnd + 1)
										);

		}
		else
			queryEnd = pathEnd;

		if (queryEnd != string::npos)	// '#' exists
			ret.fragment = reqTarget.substr(queryEnd + 1);
	}

	return ret;
}
