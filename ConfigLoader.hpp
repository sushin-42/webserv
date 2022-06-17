#ifndef CONFIGLOADER_HPP
# define CONFIGLOADER_HPP

#include "Config.hpp"
#include "ServerConfig.hpp"
# include <map>
#include <string>
using namespace std;

class ServerSocket;
bool  isNotHostChar(char c);
class ConfigLoader
{
/**========================================================================
* '                              typedefs
*========================================================================**/
private:
	typedef	pair<string, unsigned short>	_Addr;
	typedef	vector<Config*>					_Confs;
	typedef	map<_Addr, _Confs>				_Map;

/**========================================================================
* %                          member variables
*========================================================================**/
private:
	static ConfigLoader*	conf;
	_Map					addrs;

/**========================================================================
* @                           Constructors
*========================================================================**/

private:
	ConfigLoader() : addrs() {}
	~ConfigLoader() {}

/**========================================================================
* #                          member functions
*========================================================================**/
public:
	static ConfigLoader*	_()
	{
		if (conf == NULL) conf = new ConfigLoader;
		return conf;
	}


	ServerConfig*	getDefaultServer(ServerSocket* serv) const;
	ServerConfig*	getDefaultServer(const string& ip, unsigned short port) const;

	/* return server matched with host, or default server */
	ServerConfig*	getMatchedServer(ServerSocket* serv, const string& host) const;
	ServerConfig*	getMatchedServer(const string& ip, unsigned short port, const string& host) const;

	/* return LocationConfig matched with URI, or ServerConfig */
	Config*			getMatchedLocation(const string& URI, const string& host) const;

private:
	string			validateHost(const string& host)
	{
		string	ret;
		string::size_type	pos;

		switch (pos = host.find("http://"))
		{
		case 0:				ret = host.substr(7); break;
		case string::npos:	ret = host.substr(7); break;
		default:			return "";
		}

		if (find_if(ret.begin(), ret.end(), isNotHostChar) == ret.end())
			return ret;
		return "";
	}
};

bool			isNotHostChar(char c)
{
	if (c <= 0x20 || c == 0x7f || c == '\\' || c == '/')
		return true;
	return false;
}
ConfigLoader* ConfigLoader::conf;

#endif


