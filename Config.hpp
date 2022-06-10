#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <unistd.h>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "utils.hpp"
#include "ConfigUtils.hpp"
using namespace std;

string root = getcwd(NULL, 0);

map<string, string> MIME = getMIME();

// HttpConfig http;

/* in main context */
// map<string, string>	env;
// string				user;
// string				working_directory;

// COMMON_CONFIG_TOKEN = {
// "root",
// "auto_index",
// "error_page",
// "keepalive_requests",
// "default_type",
// "client_body_size",
// "reset_timedout_connection",
// "lingering_timeout",
// "lingering_time",
// "keepalive_time",
// "keepalive_timeout",
// "send_timeout",
// "client_body_timeout"
// };

typedef void (*PointerFunction)(vector<string> arg);
typedef map<std::string, PointerFunction> func_map;
class ErrorPage
{
};
class Config
{
	/**========================================================================
	 * %                          member variables
	 *========================================================================**/

protected:
	vector<Config *> link;

	func_map m;

	vector<string> conf;
	string configtemp;
	string root;
	bool auto_index; // directory listing
	ErrorPage error_page;
	int keepalive_requests;

	string default_type;
	int client_body_size;
	bool reset_timedout_connection;

	time_t lingering_timeout;
	time_t lingering_time;
	time_t keepalive_time;
	time_t keepalive_timeout;
	time_t send_timeout;
	time_t client_body_timeout;

	// bool				absolute_redirect;
	// bool				server_name_in_redirect;
	// bool				port_in_redirect;

	/**========================================================================
	* @                           Constructors

	*========================================================================**/
public:
	Config() : link() { MapSetting(); }
	Config(const Config &src) : link(src.link) {}
	virtual ~Config() {}

	/**========================================================================
	 * *                            operators
	 *========================================================================**/

	Config &operator=(const Config &src)
	{
		if (this != &src)
		{
			this->link = src.link;
		}
		return *this;
	}

	/**========================================================================
	 * #                          member functions
	 *========================================================================**/
	vector<Config *> getLink()
	{
		return this->link;
	}
	void MapSetting()
	{
		m["root"] = &parse_root;
		m["listen"] = &parse_listen;
		m["server_name"] = &parse_server_name;
		m["index"] = &parse_index;
	}
	int call_function(const std::string &pFunction, const vector<string> arg)
	{
		func_map::iterator it;

		if ((it = m.find(pFunction)) != m.end())
			(*it->second)(arg);
		else
			return -1; // 맞는거 없어서 에러 내뱉기 구현
		return 0;
	}

	void SetupConfig()
	{
		string line;
		stringstream ss(configtemp);
		while (getline(ss, line, ';'))
			conf.push_back(line);
		for (size_t i = 0; i < conf.size(); i++)
		{
			stringstream ss(conf[i]);
			string directive, tmp;
			vector<string> arg;

			ss.str(conf[i]);
			ss >> directive;
			while (ss >> tmp)
				arg.push_back(tmp);

			call_function(directive, arg);
		}
	}
	string ExtractBlock(string &configtemp, size_t start)
	{
		int countBracket = 1;
		string extractConfig;
		size_t i;
		size_t end;

		i = configtemp.find('{', start);

		for (end = i + 1; end < configtemp.length(); end++)
		{
			if (configtemp[end] == '{')
				countBracket += 1;
			else if (configtemp[end] == '}')
				countBracket -= 1;
			if (countBracket == 0)
				break;
			extractConfig.push_back(configtemp[end]);
		}
		configtemp.erase(start, end - start + 1);
		return extractConfig;
	}
	/**========================================================================
	 * !                            Exceptions
	 *========================================================================**/
};

//*--------------------------------------------------------------------------*//

#endif
