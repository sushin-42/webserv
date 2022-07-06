#ifndef CONFIGLOADER_HPP
# define CONFIGLOADER_HPP

# include "Config.hpp"
# include "LocationConfig.hpp"
# include "ServerConfig.hpp"
# include "ServerSocket.hpp"
# include "utils.hpp"
# include <map>
# include <string>

# define CONF	ConfigLoader::_()
using namespace std;

class ConfigLoader
{
/**========================================================================
* '                              typedefs
*========================================================================**/
private:
	typedef	pair<string, unsigned short>	_Addr;
	typedef	vector<Config*>					_Confs;
	typedef	map<_Addr, _Confs>				_Map;
	typedef	map<string, string>				_MIME;

/**========================================================================
* %                          member variables
*========================================================================**/
private:
	static ConfigLoader*	conf;
	_Map					addrs;
public:
	_MIME					MIME;


/**========================================================================
* @                           Constructors
*========================================================================**/

private:
	ConfigLoader();
public:
	~ConfigLoader();

/**========================================================================
* #                          member functions
*========================================================================**/
public:
	static ConfigLoader*	_()
	{
		if (conf == NULL)
		{
			conf = new ConfigLoader;
		}
		return conf;
	}

	void			loadMIME();
	// const _MIME&	getMIME() const;

	void			setAddrs(const _Map& m);
	_Map&			getAddrs();
	void			pritAddrs();

	ServerConfig*	getDefaultServer(ServerSocket* serv) const;
	ServerConfig*	getDefaultServer(const string& ip, unsigned short port) const;
	/* return server matched with host, or default server */
	ServerConfig*	getMatchedServer(ServerSocket* serv, const string& host) const;
	ServerConfig*	getMatchedServer(const string& ip, unsigned short port, const string& host) const;

	/* return LocationConfig matched with URI, or ServerConfig */
	Config*			getMatchedLocation(const string& URI, ServerConfig* servConf) const;

	string			getServerName(ServerSocket* serv, const string& host) const;
};

bool			isNotHostChar(char c);
string			validateHost(const string& host);
#endif


