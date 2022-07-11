#include "ConfigLoader.hpp"

ConfigLoader*	ConfigLoader::conf;

/**========================================================================
* @                           Constructors
*========================================================================**/

ConfigLoader::ConfigLoader() : addrs() {}
ConfigLoader::~ConfigLoader() {}

/**========================================================================
* #                          member functions
*========================================================================**/

// const ConfigLoader::_MIME&	ConfigLoader::getMIME() const			{ return MIME; }
void						ConfigLoader::setAddrs(const _Map& m)	{ this->addrs = m; }
_Map&						ConfigLoader::getAddrs() 				{ return this->addrs; }
void						ConfigLoader::pritAddrs()
{
	_Map::iterator it = addrs.begin();
	_Map::iterator ite = addrs.end();

	for(;it != ite; it++)
	{
		cout << "[" << (it->first.first) << ":" << it->first.second  << "]" << endl;
		_Confs v = it->second;
		_Confs::iterator vit = v.begin();
		_Confs::iterator vite = v.end();

		cout << "{" << endl;
		for (;vit!=vite;vit++)
		{
			ServerConfig* s = CONVERT((*vit), ServerConfig);
			if (s)
			{
				vector<string>::iterator sit = s->server_names.begin();
				vector<string>::iterator site = s->server_names.end();
				cout << "\tserver name: ";
				for (;sit!=site;sit++)
					cout << *sit << " ";
				cout << endl;
			}
		}
		cout << "}" << endl;
	}
}

void		ConfigLoader::loadMIME()
{
	string content = fileToString("./mime.types");
	string type;
	string ext;
	// string::size_type found = 0;
	string::size_type typestart = 0;
	string::size_type typeend = 0;
	string::size_type extstart = 0;
	string::size_type extend = 0;
	while (1)
	{
		typestart = content.find_first_not_of(" \t\n;", extend);
		typeend = content.find_first_of(" \t\n", typestart + 1);
		if (typestart == string::npos)
			break;

		type = content.substr(typestart, typeend - typestart);
		extend = content.find('|', typeend + 1);
		while (content[extend] != ';')
		{
			extstart = content.find_first_not_of(" \t", extend + 1);
			extend = content.find_first_of(" \t;", extstart + 1);
			ext = content.substr(extstart, extend - extstart);
			MIME[ext] = type;
		}
}

}

ServerConfig*	ConfigLoader::getDefaultServer(ServerSocket* serv) const
{
	return CONVERT(serv->confs[0], ServerConfig);
}
ServerConfig*	ConfigLoader::getDefaultServer(const string& ip, unsigned short port) const
{
	_Addr	a(ip, port);
	_Confs	v = addrs.find(a)->second;

	return CONVERT(v[0], ServerConfig);
}

/* return server matched with host, or default server */
ServerConfig*	ConfigLoader::getMatchedServer(ServerSocket* serv, const string& host) const
{
	string	Host = validateHost(host);
	_Confs	v = serv->confs;
	_Confs::iterator it, ite;
	it = v.begin(), ite = v.end();

	for (; it < ite ; it++)
	{
		ServerConfig* servConf = CONVERT(*it, ServerConfig);
		vector<string> names = servConf->server_names;
		vector<string>::iterator itName, iteName;
		itName = names.begin(), iteName = names.end();
		for (; itName < iteName ; itName++)
		{
			if (itName->compare(Host) == 0 ||
				(*itName + ":" + toString(serv->getPort())).compare(Host) == 0)

				return servConf;
		}
	}
	return CONVERT(v[0], ServerConfig);
}

ServerConfig*	ConfigLoader::getMatchedServer(const string& ip, unsigned short port, const string& host) const
{
	string	Host = validateHost(host);
	_Addr	a(ip, port);
	_Confs	v = addrs.find(a)->second;
	_Confs::iterator it, ite;
	it = v.begin(), ite = v.end();

	for (; it < ite ; it++)
	{
		ServerConfig* servConf = CONVERT(*it, ServerConfig);
		vector<string> names = servConf->server_names;
		vector<string>::iterator itName, iteName;
		itName = names.begin(), iteName = names.end();
		for (; itName < iteName ; itName++)
		{
			if (itName->compare(Host) == 0 ||
				(*itName + ":" + toString(port)).compare(Host) == 0)
				return servConf;
		}
	}
	return CONVERT(v[0], ServerConfig);
}

/* return LocationConfig matched with URI, or ServerConfig */
Config*			ConfigLoader::getMatchedLocation(const string& URI, ServerConfig* servConf) const
{
	LocationConfig*		locConf;
	_Confs				locConfs = servConf->link;
	_Confs::iterator	it, ite;
	string				locURI;
	string::size_type	locURIlen;
	string::size_type	URIlen = URI.length();

	string::size_type	max = 0;
	Config*				mostMatched = servConf;

	it = locConfs.begin(), ite = locConfs.end();

	for (; it < ite ; it++)
	{
		locConf = CONVERT(*it, LocationConfig);
		locURI = locConf->URI;
		locURIlen = locURI.length();
		if (locURIlen > URIlen)
			continue;

		if ( URI.compare(0, locURIlen, locURI) == 0 )
		{
			if (locConf->assign)
				return locConf;
			if (locURIlen > max)
			{
				max = locURIlen;
				mostMatched = locConf;
			}
		}
	}
	return mostMatched;
}

bool			isNotHostChar(char c)
{
	if (c <= 0x20 || c == 0x7f || c == '\\' || c == '/')
		return true;
	return false;
}
string			validateHost(const string& host)
{
	string	ret;
	string::size_type	pos;

	switch (pos = host.find("http://"))
	{
	case 0:				ret = host.substr(7); break;
	case string::npos:	ret = host; break;
	default:			return "";
	}

	if (find_if(ret.begin(), ret.end(), isNotHostChar) == ret.end())
		return ret;
	return "";
}

string			ConfigLoader::getServerName(ServerSocket* serv, const string& host) const
{
	string				Host = validateHost(host);
	_Confs				v = serv->confs;
	_Confs::iterator	it, ite;

	ServerConfig*		defaultServConf;


	it = v.begin(), ite = v.end();
	defaultServConf = CONVERT(*it, ServerConfig);
	for (; it < ite ; it++)
	{
		ServerConfig*	servConf = CONVERT(*it, ServerConfig);
		vector<string> 	names	= servConf->server_names;
		vector<string>::iterator itName, iteName;
		itName = names.begin(), iteName = names.end();
		for (; itName < iteName ; itName++)
		{
			if (itName->compare(Host) == 0 ||
				(*itName + ":" + toString(serv->getPort())).compare(Host) == 0)
				return *itName;
		}
	}
	return defaultServConf->server_names[0];
}



