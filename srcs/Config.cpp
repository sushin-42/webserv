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
        // cout << directive << ":";
        // for (size_t i = 0; i < arg.size(); i++)
        // {
        //     cout << arg[i] << ", ";
        // }
        // cout << endl;
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

//=====================================parse_arg========================================

// void Config::parse_root(vector<string> arg, Config *config)
// {
//     if (arg.size() != 1 || config->dupeCheck.root == true)
//         throw Config::parseRootFail();
//     isPath(arg[0]);
//     config->root = arg[0];
//     config->dupeCheck.root = true;
// }

// void Config::parse_listen(vector<string> arg, Config *config)
// {

//     string::size_type pos;
//     unsigned short port;
//     string ip = "";

//     if (arg.size() != 1)
//         throw Config::parseListenFail();
//     ServerConfig *server = dynamic_cast<ServerConfig *>(config);
//     if ((pos = arg[0].find(':')) != string::npos)
//     {
//         ip = convertStringToIP(arg[0].substr(0, pos));
//         port = convertStringToPort(arg[0].substr(pos + 1, arg[0].length()));
//     }
//     else
//     {
//         port = convertStringToPort(arg[0]);
//         ip = "127.0.0.1";
//     }

//     for (vector<_Addr>::size_type i = 0; i < server->ipPort.size(); i++)
//     {
//         if (server->ipPort[i].first == ip && server->ipPort[i].second == port)
//             throw Config::parseListenFail();
//     }
//     server->ipPort.push_back(make_pair(ip, port));
// }

// void Config::parse_server_names(vector<string> arg, Config *config)
// {
//     ServerConfig *server = dynamic_cast<ServerConfig *>(config);

//     for (vector<string>::size_type i = 0; i < arg.size(); i++)
//         server->server_names.push_back(arg[i]);
// }

// void Config::parse_index(vector<string> arg, Config *config)
// {
//     for (vector<string>::size_type i = 0; i < arg.size(); i++)
//     {
//         isPath(arg[i]);
//         config->index.push_back(arg[i]);
//     }
//     config->dupeCheck.index = true;
// }

// void Config::parse_auto_index(vector<string> arg, Config *config)
// {

//     string lowString;

//     if (arg.size() != 1 || config->dupeCheck.autoindex == true)
//         throw Config::parseAutoIndexFail();
//     lowString = convertStringToLower(arg[0]);
//     if (lowString == "off")
//         config->auto_index = false;
//     else if (lowString == "on")
//         config->auto_index = true;
//     else
//         throw Config::parseAutoIndexFail();
//     config->dupeCheck.autoindex = true;
// }

// void Config::parse_error_page(vector<string> arg, Config *config)
// {
//     string path;
//     int status;

//     if (arg.size() < 2)
//         throw Config::parseErrorPageFail();
//     isPath(arg[arg.size() - 1]);
//     path = arg[arg.size() - 1];

//     for (vector<string>::size_type i = 0; i < (arg.size() - 1); i++)
//     {
//         status = convertStringToStateCode(arg[i]);
//         config->error_page[status] = path;
//     }
// }

// void Config::parse_keepalive_requests(vector<string> arg, Config *config)
// {
//     if (arg.size() != 1 || config->dupeCheck.keepalive_requests == true)
//         throw Config::parseKeepRequestsFail();
//     config->keepalive_requests = convertStringToSsize_T(arg[0]);
//     config->dupeCheck.keepalive_requests = true;
// }

// void Config::parse_default_type(vector<string> arg, Config *config)
// {
//     if (arg.size() != 1 || config->dupeCheck.default_type == true)
//         throw Config::parseDefaultTypeFail();
//     config->default_type = arg[0];
//     config->dupeCheck.default_type = true;
// }

// void Config::parse_client_max_body_size(vector<string> arg, Config *config)
// {
//     if (arg.size() != 1 || config->dupeCheck.client_max_body_size == true)
//         throw Config::parseClientBodySizeFail();
//     config->client_max_body_size = convertStringToByte(arg[0]);
//     config->dupeCheck.client_max_body_size = true;
// }

// void Config::parse_reset_timedout_connection(vector<string> arg, Config *config)
// {
//     string button;

//     if (arg.size() != 1 || config->dupeCheck.reset_timedout_connection == true)
//         throw Config::parseResetTimedoutConnFail();
//     button = convertStringToLower(arg[0]);
//     if (button == "off")
//         config->reset_timedout_connection = 0;
//     else
//         config->reset_timedout_connection = 1;
//     config->dupeCheck.reset_timedout_connection = true;
// }

// void Config::parse_lingering_timeout(vector<string> arg, Config *config)
// {
//     if (arg.size() != 1 || config->dupeCheck.lingering_timeout == true)
//         throw Config::parseLingeringTimeoutFail();
//     config->lingering_timeout = convertStringToTime(arg[0]);
//     config->dupeCheck.lingering_timeout = true;
// }

// void Config::parse_lingering_time(vector<string> arg, Config *config)
// {
//     if (arg.size() != 1 || config->dupeCheck.lingering_time == true)
//         throw Config::parseLingeringTimeFail();
//     config->lingering_time = convertStringToTime(arg[0]);
//     config->dupeCheck.lingering_time = true;
// }

// void Config::parse_keepalive_time(vector<string> arg, Config *config)
// {
//     if (arg.size() != 1 || config->dupeCheck.keepalive_time == true)
//         throw Config::parseKeepTimeFail();
//     config->keepalive_time = convertStringToTime(arg[0]);
//     config->dupeCheck.keepalive_time = true;
// }

// void Config::parse_keepalive_timeout(vector<string> arg, Config *config)
// {
//     if (arg.size() != 1 || config->dupeCheck.keepalive_timeout == true)
//         throw Config::parseKeepTimeoutFail();
//     config->keepalive_timeout = convertStringToTime(arg[0]);
//     config->dupeCheck.keepalive_timeout = true;
// }

// void Config::parse_send_timeout(vector<string> arg, Config *config)
// {
//     if (arg.size() != 1 || config->dupeCheck.send_timeout == true)
//         throw Config::parseSendTimeoutFail();
//     config->send_timeout = convertStringToTime(arg[0]);
//     config->dupeCheck.send_timeout = true;
// }

// void parse_client_body_timeout(vector<string> arg, Config *config)
// {
//     if (arg.size() != 1 || config->dupeCheck.client_body_timeout == true)
//         throw Config::parseClientBodyTimeoutFail();
//     config->client_body_timeout = convertStringToTime(arg[0]);
//     config->dupeCheck.client_body_timeout = true;
// }

// void Config::parse_limit_except_method(vector<string> arg, Config *config)
// {
//     LocationConfig *location;
//     string method;
//     vector<string>::size_type check;

//     location = dynamic_cast<LocationConfig *>(config);
//     for (vector<string>::size_type i = 0; i < arg.size(); i++)
//     {
//         method = convertStringToUpper(arg[i]);
//         check = 0;
//         for (check = 0; check < location->limit_except_method.size(); check++)
//         {
//             if (location->limit_except_method[check] == method)
//                 break;
//         }
//         if (check == location->limit_except_method.size() && (method == "GET" || method == "POST" || method == "PUT" || method == "DELETE"))
//             location->limit_except_method.push_back(method);
//         else
//             throw parseLimitExceptMethodFail();
//     }
// }

//=====================================parse_arg========================================

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

    // only server block
    m["listen"] = &parse_listen;
    m["server_name"] = &parse_server_names;

    // only location_block
    m["limit_except_method"] = &parse_limit_except_method;
    m["alias"] = &parse_alias;
    m["file_access"] = &parse_file_access;
}
