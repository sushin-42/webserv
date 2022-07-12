#include "Config.hpp"
#include "ConfigUtils.hpp"
#include "HttpConfig.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

/**========================================================================
 * %                          member variables
 *========================================================================**/

Config::Config() : dupeCheck(), link()
{
    MapSetting();
}
Config::Config(const Config &src) : dupeCheck(), link(src.link) {}
Config::~Config() {}

/**========================================================================
 * *                            operators
 *========================================================================**/

Config &Config::operator=(const Config &src)
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
int Config::call_function(const std::string &pFunction, const vector<string> arg)
{
    func_map::iterator it;

    if ((it = m.find(pFunction)) != m.end())
        (*it->second)(arg, this);
    else
    {
        cout << "worngDirective = " << pFunction << endl;
        throw wrongDirective();
    }
    return 0;
}

void Config::SetupConfig()
{
    string line;
    stringstream ss(configtemp);
    while (getline(ss, line, ';'))
    {
        if (!ss.eof())
            conf.push_back(line);
    }
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
string Config::ExtractBlock(string &configtemp, size_t start)
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

//*--------------------------------------------------------------------------*//

void Config::MapSetting()
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
    m["cgi"] = &parse_cgi;
    m["timer"] = &parse_timer;

    // only server block
    m["listen"] = &parse_listen;
    m["server_name"] = &parse_server_names;
    m["return"] = &parse_return; // with location block

    // only location_block
    m["limit_except_method"] = &parse_limit_except_method;
    m["alias"] = &parse_alias;
    m["file_access"] = &parse_file_access;
    m["server_name_in_redirect"] = &parse_server_name_in_redirect;
    m["port_in_redirect"] = &parse_port_in_redirect;
}
