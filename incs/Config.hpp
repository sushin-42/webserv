#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <string>
#include <unistd.h>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <utility>
#include <cstddef>
#include "color.hpp"
#include "utils.hpp"
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
	bool timer;
	bool server_name_in_redirect;
	bool port_in_redirect;
	bool file_access;
	bool d_return;

	bool &operator[](int ind) { return *(&index + ind); }
};

typedef pair<string, string> _LocUri;
typedef pair<string, unsigned short> _Addr;
typedef vector<Config *> _Confs;
typedef map<_Addr, _Confs> _Map;
typedef unsigned short		status_code_t;

class Config
{
	/**========================================================================
	 * %                          member variables
	 *========================================================================**/

public:
	typedef void (*PointerFunction)(vector<string> arg, Config *config);
	typedef map<string, PointerFunction> func_map;
	//파싱 변수
	func_map m;
	vector<string> conf;
	string configtemp;
	//파싱 플래그 index, error_page
	bool handdownIndex;
	bool handdownErrorPage;
	//중복 체크 구조체
	Duplicate dupeCheck;

	//멤버 변수
	map<int, pair<int, string> > error_page; // key= status code, value = 문서
	vector<Config *> link;
	vector<string> index;		// if (directory) 1. index file 2. (403 forbidden  || listing)
	bool auto_index;			// if (directory && no index file) directory listing
	string root;				// 해당 블록의 루트 폴더. <--- 해당 path에 request Target을 붙이게 됨.
	ssize_t keepalive_requests; // keep_alive --;

	string default_type; // 확장자가 mime.type에 없을때, 기본값
	ssize_t client_max_body_size;
	bool reset_timedout_connection;
	bool file_access;

	time_t lingering_time;	  //총시간->  fin 보낸순간.
	time_t lingering_timeout; //간격 ->  lastActive 일단 fin보낸순간, 그 다음부터 폴에서 갱신
	time_t keepalive_time;	  //총시간 -> 생성시간
	time_t keepalive_timeout; //간격 -> poll에서 갱신되는 lastActive
	time_t send_timeout;
	time_t client_body_timeout;
	time_t timer;

	map<string, string> cgi;
	pair<status_code_t, string> d_return;
	bool	server_name_in_redirect;
	bool	port_in_redirect;

	/**========================================================================
	 * @                           Constructors
	 *========================================================================**/
public:
	Config();
	Config(const Config &src);
	virtual ~Config();

	/**========================================================================
	 * *                            operators
	 *========================================================================**/

	Config &operator=(const Config &src);

	/**========================================================================
	 * #                          member functions
	 *========================================================================**/

	void MapSetting();
	int call_function(const std::string &pFunction, const vector<string> arg);

	void SetupConfig();
	string ExtractBlock(string &configtemp, size_t start);
	/**========================================================================
	 * !                            Parse_utils
	 *========================================================================**/

	
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

	class parseAliasFail : public exception
	{
	private:
		string msg;

	public:
		explicit parseAliasFail() : msg(RED("parseAliasFail")) {}
		explicit parseAliasFail(const string &m) : msg(m) {}
		virtual ~parseAliasFail() throw(){};
		virtual const char *what() const throw() { return msg.c_str(); }
	};
	class parseCDflagFail : public exception
	{
	private:
		string msg;

	public:
		explicit parseCDflagFail() : msg(RED("parseCDflagFail")) {}
		explicit parseCDflagFail(const string &m) : msg(m) {}
		virtual ~parseCDflagFail() throw(){};
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

	class parseReturnFail : public exception
	{
	private:
		string msg;

	public:
		explicit parseReturnFail() : msg(RED("parseReturnFail")) {}
		explicit parseReturnFail(const string &m) : msg(m) {}
		virtual ~parseReturnFail() throw(){};
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

	class parseServerNameInRedirectFail : public exception
	{
	private:
		string msg;

	public:
		explicit parseServerNameInRedirectFail() : msg(RED("parseServerNameInRedirectFail")) {}
		explicit parseServerNameInRedirectFail(const string &m) : msg(m) {}
		virtual ~parseServerNameInRedirectFail() throw(){};
		virtual const char *what() const throw() { return msg.c_str(); }
	};

	class parsePortInRedirectFail : public exception
	{
	private:
		string msg;

	public:
		explicit parsePortInRedirectFail() : msg(RED("parsePortInRedirectFail")) {}
		explicit parsePortInRedirectFail(const string &m) : msg(m) {}
		virtual ~parsePortInRedirectFail() throw(){};
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
	class parseTimerFail : public exception
	{
	private:
		string msg;

	public:
		explicit parseTimerFail() : msg(RED("parseTimerFail")) {}
		explicit parseTimerFail(const string &m) : msg(m) {}
		virtual ~parseTimerFail() throw(){};
		virtual const char *what() const throw() { return msg.c_str(); }
	};
	class parseCgiFail : public exception
	{
	private:
		string msg;

	public:
		explicit parseCgiFail() : msg(RED("parseCgiFail")) {}
		explicit parseCgiFail(const string &m) : msg(m) {}
		virtual ~parseCgiFail() throw(){};
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
	class wrongDirective : public exception
	{
	private:
		string msg;

	public:
		explicit wrongDirective() : msg(RED("wrongDirective")) {}
		explicit wrongDirective(const string &m) : msg(m) {}
		virtual ~wrongDirective() throw(){};
		virtual const char *what() const throw() { return msg.c_str(); }
	};
};

//*--------------------------------------------------------------------------*//

#endif
