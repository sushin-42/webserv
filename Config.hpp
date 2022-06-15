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
// #include "ConfigUtils.hpp"
using namespace std;
class Config;
struct Duplicate
{
	bool index;
	bool root;
	bool autoindex;
	bool keepalive_time;
	bool keepalive_timeout;
	bool keepalive_requests;
	bool default_type;
	bool client_max_body_size;
	bool reset_timedout_connection;
	bool lingering_time;
	bool lingering_timeout;
	bool send_timeout;
	bool client_body_timeout;

	bool &operator[](int index) { return *(&root + index); }
};
// ssize_t convertStringToByte(string val);
// time_t convertStringToTime(string val);
// unsigned short convertStringToPort(string code);

void parse_root(vector<string> arg, Config *config);
void parse_listen(vector<string> arg, Config *config);
void parse_server_name(vector<string> arg, Config *config);
void parse_index(vector<string> arg, Config *config);
void parse_index(vector<string> arg, Config *config);
void parse_root(vector<string> arg, Config *config);
void parse_auto_index(vector<string> arg, Config *config);
void parse_error_page(vector<string> arg, Config *config);
void parse_keepalive_requests(vector<string> arg, Config *config);
void parse_keepalive_time(vector<string> arg, Config *config);
void parse_keepalive_timeout(vector<string> arg, Config *config);
void parse_lingering_time(vector<string> arg, Config *config);
void parse_lingering_timeout(vector<string> arg, Config *config);
void parse_send_timeout(vector<string> arg, Config *config);
void parse_client_body_timeout(vector<string> arg, Config *config);
void parse_reset_timedout_connection(vector<string> arg, Config *config);
void parse_client_max_body_size(vector<string> arg, Config *config);
void parse_default_type(vector<string> arg, Config *config);
void parse_listen(vector<string> arg, Config *config);
void parse_server_name(vector<string> arg, Config *config);
void parse_limit_except_method(vector<string> arg, Config *config);
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
// "client_max_body_size",
// "reset_timedout_connection",
// "lingering_timeout",
// "lingering_time",
// "keepalive_time",
// "keepalive_timeout",
// "send_timeout",
// "client_body_timeout"
// };
typedef void (*PointerFunction)(vector<string> arg, Config *config);
typedef map<std::string, PointerFunction> func_map;
class ErrorPage
{
};
class Config
{
	/**========================================================================
	 * %                          member variables
	 *========================================================================**/

public:
	//파싱 변수
	func_map m;
	vector<string> conf;
	string configtemp;
	//중복 체크 구조체
	Duplicate dupeCheck;
	//멤버 변수
	vector<Config *> link;
	vector<string> index;
	string root;
	bool auto_index; // directory listing
	map<int, string> error_page;
	ssize_t keepalive_requests;

	string default_type;
	size_t client_max_body_size;
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
	Config() : link()
	{
		MapSetting();
		keepalive_requests = 0;
		client_max_body_size = 0;;
		lingering_timeout = 0;
		lingering_time = 0;
		keepalive_time = 0;
		keepalive_timeout = 0;
		send_timeout = 0;
		client_body_timeout = 0;
	}
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

	void MapSetting()
	{
		m["index"] = &parse_index;
		m["root"] = &parse_root;
		m["auto_index"] = &parse_auto_index;
		m["error_page"] = &parse_error_page;
		m["keepalive_requests"] = &parse_keepalive_requests;
		m["keepalive_time"] = &parse_keepalive_time;
		m["keepalive_timeout"] = &parse_keepalive_timeout;
		m["lingering_time"] = &parse_lingering_time;
		m["lingering_timeout"] = &parse_lingering_timeout;
		m["send_timeout"] = &parse_send_timeout;
		m["client_body_timeout"] = &parse_client_body_timeout;
		m["reset_timedout_connection"] = &parse_reset_timedout_connection;
		m["client_max_body_size"] = &parse_client_max_body_size;
		m["default_type"] = &parse_default_type;

		// only server block
		m["listen"] = &parse_listen;
		m["server_name"] = &parse_server_name;

		// only location_block
		m["limit_except_method"] = &parse_limit_except_method;
	}
	int call_function(const std::string &pFunction, const vector<string> arg)
	{
		func_map::iterator it;

		if ((it = m.find(pFunction)) != m.end())
			(*it->second)(arg, this);
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

	class parseFail : public exception
	{
	private:
		string msg;

	public:
		explicit parseFail() : msg(RED("parseFail")) {}
		explicit parseFail(const string &m) : msg(m) {}
		virtual ~parseFail() throw(){};
		virtual const char *what() const throw() { return msg.c_str(); }
	};

	class parseRootFail : public exception
	{
	private:
		string msg;

	public:
		explicit parseRootFail() : msg(RED("parseRootFail")) {}
		explicit parseRootFail(const string &m) : msg(m) {}
		virtual ~parseRootFail() throw(){};
		virtual const char *what() const throw() { return msg.c_str(); }
	};

	class parseListenFail : public exception
	{
	private:
		string msg;

	public:
		explicit parseListenFail() : msg(RED("parseListenFail")) {}
		explicit parseListenFail(const string &m) : msg(m) {}
		virtual ~parseListenFail() throw(){};
		virtual const char *what() const throw() { return msg.c_str(); }
	};

	class parseServerNameFail : public exception
	{
	private:
		string msg;

	public:
		explicit parseServerNameFail() : msg(RED("parseServerNameFail")) {}
		explicit parseServerNameFail(const string &m) : msg(m) {}
		virtual ~parseServerNameFail() throw(){};
		virtual const char *what() const throw() { return msg.c_str(); }
	};

	class parseAutoIndexFail : public exception
	{
	private:
		string msg;

	public:
		explicit parseAutoIndexFail() : msg(RED("parseAutoIndexFail")) {}
		explicit parseAutoIndexFail(const string &m) : msg(m) {}
		virtual ~parseAutoIndexFail() throw(){};
		virtual const char *what() const throw() { return msg.c_str(); }
	};

	class parseErrorPageFail : public exception
	{
	private:
		string msg;

	public:
		explicit parseErrorPageFail() : msg(RED("parseErrorPageFail")) {}
		explicit parseErrorPageFail(const string &m) : msg(m) {}
		virtual ~parseErrorPageFail() throw(){};
		virtual const char *what() const throw() { return msg.c_str(); }
	};

	class parseKeepRequestsFail : public exception
	{
	private:
		string msg;

	public:
		explicit parseKeepRequestsFail() : msg(RED("parseKeepRequestsFail")) {}
		explicit parseKeepRequestsFail(const string &m) : msg(m) {}
		virtual ~parseKeepRequestsFail() throw(){};
		virtual const char *what() const throw() { return msg.c_str(); }
	};

	class parseDefaultTypeFail : public exception
	{
	private:
		string msg;

	public:
		explicit parseDefaultTypeFail() : msg(RED("parseDefaultTypeFail")) {}
		explicit parseDefaultTypeFail(const string &m) : msg(m) {}
		virtual ~parseDefaultTypeFail() throw(){};
		virtual const char *what() const throw() { return msg.c_str(); }
	};

	class parseClientBodySizeFail : public exception
	{
	private:
		string msg;

	public:
		explicit parseClientBodySizeFail() : msg(RED("parseClientBodySizeFail")) {}
		explicit parseClientBodySizeFail(const string &m) : msg(m) {}
		virtual ~parseClientBodySizeFail() throw(){};
		virtual const char *what() const throw() { return msg.c_str(); }
	};

	class parseResetTimedoutConnFail : public exception
	{
	private:
		string msg;

	public:
		explicit parseResetTimedoutConnFail() : msg(RED("parseResetTimedoutConnFail")) {}
		explicit parseResetTimedoutConnFail(const string &m) : msg(m) {}
		virtual ~parseResetTimedoutConnFail() throw(){};
		virtual const char *what() const throw() { return msg.c_str(); }
	};

	class parseLingeringTimeFail : public exception
	{
	private:
		string msg;

	public:
		explicit parseLingeringTimeFail() : msg(RED("parseLingeringTimeFail")) {}
		explicit parseLingeringTimeFail(const string &m) : msg(m) {}
		virtual ~parseLingeringTimeFail() throw(){};
		virtual const char *what() const throw() { return msg.c_str(); }
	};

	class parseLingeringTimeoutFail : public exception
	{
	private:
		string msg;

	public:
		explicit parseLingeringTimeoutFail() : msg(RED("parseLingeringTimeoutFail")) {}
		explicit parseLingeringTimeoutFail(const string &m) : msg(m) {}
		virtual ~parseLingeringTimeoutFail() throw(){};
		virtual const char *what() const throw() { return msg.c_str(); }
	};

	class parseKeepTimeFail : public exception
	{
	private:
		string msg;

	public:
		explicit parseKeepTimeFail() : msg(RED("parseKeepTimeFail")) {}
		explicit parseKeepTimeFail(const string &m) : msg(m) {}
		virtual ~parseKeepTimeFail() throw(){};
		virtual const char *what() const throw() { return msg.c_str(); }
	};

	class parseKeepTimeoutFail : public exception
	{
	private:
		string msg;

	public:
		explicit parseKeepTimeoutFail() : msg(RED("parseKeepTimeoutFail")) {}
		explicit parseKeepTimeoutFail(const string &m) : msg(m) {}
		virtual ~parseKeepTimeoutFail() throw(){};
		virtual const char *what() const throw() { return msg.c_str(); }
	};

	class parseSendTimeoutFail : public exception
	{
	private:
		string msg;

	public:
		explicit parseSendTimeoutFail() : msg(RED("parseSendTimeoutFail")) {}
		explicit parseSendTimeoutFail(const string &m) : msg(m) {}
		virtual ~parseSendTimeoutFail() throw(){};
		virtual const char *what() const throw() { return msg.c_str(); }
	};

	class parseClientBodyTimeoutFail : public exception
	{
	private:
		string msg;

	public:
		explicit parseClientBodyTimeoutFail() : msg(RED("parseClientBodyTimeoutFail")) {}
		explicit parseClientBodyTimeoutFail(const string &m) : msg(m) {}
		virtual ~parseClientBodyTimeoutFail() throw(){};
		virtual const char *what() const throw() { return msg.c_str(); }
	};

	class parseLocationFail : public exception
	{
	private:
		string msg;

	public:
		explicit parseLocationFail() : msg(RED("parseLocationFail")) {}
		explicit parseLocationFail(const string &m) : msg(m) {}
		virtual ~parseLocationFail() throw(){};
		virtual const char *what() const throw() { return msg.c_str(); }
	};

	class httpDupe : public exception
	{
	private:
		string msg;

	public:
		explicit httpDupe() : msg(RED("httpDupe")) {}
		explicit httpDupe(const string &m) : msg(m) {}
		virtual ~httpDupe() throw(){};
		virtual const char *what() const throw() { return msg.c_str(); }
	};
	class notExistHttpBlock : public exception
	{
	private:
		string msg;

	public:
		explicit notExistHttpBlock() : msg(RED("notExistHttpBlock")) {}
		explicit notExistHttpBlock(const string &m) : msg(m) {}
		virtual ~notExistHttpBlock() throw(){};
		virtual const char *what() const throw() { return msg.c_str(); }
	};
};

//*--------------------------------------------------------------------------*//

#endif
