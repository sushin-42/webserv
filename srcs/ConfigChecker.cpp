#include "ConfigChecker.hpp"
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

bool	ConfigChecker::isAllowed(Config* conf, const string& method)
{
	_Loc* locConf = CONVERT(conf, _Loc);
	if (locConf == NULL)	return true;

	vector<string>				allowed = locConf->limit_except_method;
	vector<string>::iterator	itMethod, iteMethod;

	itMethod = allowed.begin(), iteMethod = allowed.end();

	for (; itMethod != iteMethod ; itMethod++)
	{
		if (*itMethod == method)
			return true;
	}
	return false;
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
