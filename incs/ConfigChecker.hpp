#ifndef CONFIGCHECKER_HPP
# define CONFIGCHECKER_HPP

# include "ServerConfig.hpp"
# include "LocationConfig.hpp"
#include <string>

# define CHECK ConfigChecker::_()

class ConfigChecker
{
/**========================================================================
* '                              typedefs
*========================================================================**/

	typedef ServerConfig		_Serv;
	typedef LocationConfig		_Loc;

/**========================================================================
* %                          member variables
*========================================================================**/
private:
	static ConfigChecker*	checker;

/**========================================================================
* @                           Constructors
*========================================================================**/
private:
	ConfigChecker();

public:
	~ConfigChecker();

/**========================================================================
* #                          member functions
*========================================================================**/
public:
	static ConfigChecker*	_()
	{
		if (checker == NULL) checker = new ConfigChecker;
		return checker;
	}

	bool				isAllowed(Config* conf, const string& method);
	string				getAlias(Config* conf);
	string				replaceURI(const string& reqTarget, const string& locURI, const string& alias);
	string				trimLocationURI(const string& reqTarget, const string& locURI);
	pair<string,string>	routeRequestTarget(Config* conf, const string& reqTarget);
	string				getFileName(Config* conf, const string& reqTarget);
};

#endif
