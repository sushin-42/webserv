#include "ConfigChecker.hpp"
#include "Config.hpp"
#include "HTTP_Error.hpp"
#include <string>

ConfigChecker*	ConfigChecker::checker;
/**========================================================================
* @                           Constructors
*========================================================================**/

ConfigChecker::ConfigChecker() {}
ConfigChecker::~ConfigChecker() {}

/**========================================================================
* #                          member functions
*========================================================================**/

string	ConfigChecker::getFileName(Config* conf, const string& reqTarget)
{
	string			indexfile;
	string			prefix;
	string			uri;

	pair <string, string> p = CHECK->routeRequestTarget(conf, reqTarget);
	prefix = p.first;
	uri = p.second;

	string			filename = prefix + uri;

	return filename;
}

bool	ConfigChecker::isForbiddenMethod(Config* conf, const string& method)
{
	_Loc* locConf = CONVERT(conf, _Loc);
	if (locConf == NULL)	return true;

	vector<string>				allowed = locConf->limit_except_method;
	vector<string>::iterator	itMethod, iteMethod;

	itMethod = allowed.begin(), iteMethod = allowed.end();

	for (; itMethod != iteMethod ; itMethod++)
	{
		if (*itMethod == method)
			return false;
	}
	return true;
}

string	ConfigChecker::getAlias(Config* conf)
{
	_Loc* locConf = CONVERT(conf, _Loc);
	if (locConf == NULL)	return "";
	return locConf->alias;
}

string	ConfigChecker::replaceURI(const string& reqTarget, const string& locURI, const string& alias)
{
	string ret = reqTarget;
	ret.replace(0, locURI.length(), alias);

	return ret;
}

string	ConfigChecker::trimLocationURI(const string& reqTarget, const string& locURI)
{
	string ret;

	ret = reqTarget.substr(locURI.length());
	return ret;
}

pair<string, string>	ConfigChecker::routeRequestTarget(Config* conf, const string& reqTarget)
{
	string prefix;
	string uri;
	string alias = getAlias(conf);
	if (alias.empty())
	{
		prefix = conf->root;
		uri = reqTarget;
	}
	else
	{
		LocationConfig* locConf = CONVERT(conf, LocationConfig);
		prefix = locConf->alias;
		uri = trimLocationURI(reqTarget, locConf->URI);
	}
	return make_pair(prefix, uri);
}

string				ConfigChecker::getCGIexcutable(Config* conf, const string& ext)
{
	map<string, string>::iterator it;

	it = conf->cgi.find(ext);
	if (it != conf->cgi.end())
		return it->second;
	return "";
}

void	ConfigChecker::externalRedirect(Config* conf, const string& host, short port, const string& serverName)
{
	status_code_t	status		= conf->d_return.first;
	string			loc			= conf->d_return.second;
	string			scheme		= "http://";
	string			authority	= host;

	if (loc.empty() == false && loc[0] == '/')
	{
		if (conf->server_name_in_redirect)
		{
			authority = serverName;
			if (conf->port_in_redirect)
				authority += (":" + toString(port));
		}
		loc = "http://" + authority + loc;
	}

	switch (status) {
	case 301: throw movedPermanently(loc);
	case 302: throw found(loc);
	case 303: throw seeOther(loc);
	case 307: throw temporaryRedirect(loc);
	case 308: throw permanentRedirect(loc);
	case 0:	return ;
	}

}
