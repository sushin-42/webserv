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

string	ConfigChecker::getAliasOrRoot(Config* conf)
{
	string	ret = getAlias(conf);
	if (ret.empty())
		ret = conf->root;
	return ret;
}
