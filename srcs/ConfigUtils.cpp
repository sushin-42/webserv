#include "ConfigUtils.hpp"
using namespace std;

typedef pair<string, unsigned short> _Addr;
// typedef vector<Config *> _Confs;
// typedef map<_Addr, _Confs> _Map;
//=====================================parse_util========================================
void isPath(string path)
{
    if (path.find("$") != string::npos)
        throw isPathFail();
}

string convertStringToLower(string str)
{
    string lower = "";

    for (string::size_type i = 0; i < str.length(); i++)
        lower.append(1, tolower(str[i]));
    return lower;
}

string convertStringToUpper(string str)
{
    string upper = "";

    for (string::size_type i = 0; i < str.length(); i++)
        upper.append(1, toupper(str[i]));
    return upper;
}

ssize_t convertStringToSsize_T(string val)
{
    ssize_t num = 0;
    istringstream iss;

    if (val.find_first_not_of("0123456789") != string::npos)
        throw convertStringToSsize_TFail();
    iss.str(val);
    iss >> num;
    if (iss.fail())
        throw convertStringToSsize_TFail();
    return num;
}

int convertStringToStateCode(string code)
{
    ssize_t val = 0;

    val = convertStringToSsize_T(code);
    if (!(300 <= val && val < 600))
        throw convertStringToStateCodeFail();
    return static_cast<int>(val);
}

string convertStringToIpv4(unsigned int ip)
{
    stringstream ss;
    string hexString;
    string cut;
    int pos;
    string ipv4 = "";
    int decimal;

    ss << hex << ip;
    hexString = ss.str();
    pos = static_cast<int>(hexString.length());
    for (int i = 0; i < 4; i++)
    {
        ss.str("");
        pos -= 2;
        if (pos == -1)
            decimal = strtol(hexString.c_str(), NULL, 16);
        else if (pos >= 0)
        {
            cut = hexString.substr(pos, 2);
            cout << cut << endl;
            decimal = strtol(cut.c_str(), NULL, 16);
        }
        else
            decimal = 0;
        ss << "." << dec << decimal;
        ipv4.insert(0, ss.str());
    }
    ipv4.erase(0, 1);
    return ipv4;
}

string convertStringToIP(string ip)
{
    ssize_t numString;
    int dot;

    if ((ip.find(".")) != string::npos)
    {
        dot = 0;
        for (string::size_type i = 0; i < ip.length(); i++)
        {
            if (ip[i] == '.')
                dot++;
        }
        if (inet_addr(ip.c_str()) != (in_addr_t)-1)
        {
            for (dot = 3 - dot; dot > 0; dot--)
                ip.insert(0, "0.");
            return ip;
        }
        throw convertStringToIPFail();
    }

    numString = convertStringToSsize_T(ip);
    return convertStringToIpv4(static_cast<unsigned int>(numString));
}

unsigned short convertStringToPort(string code)
{
    ssize_t val = 0;

    val = convertStringToSsize_T(code);
    if (!(0 <= val && val <= 65535))
        throw convertStringToPortFail();
    return static_cast<unsigned short>(val);
}

pair<ssize_t, string> splitStringToType(string val)
{
    string::size_type pos;
    string numStr = "";
    string type = "";
    ssize_t num;
    istringstream iss;

    pos = val.find_first_not_of("0123456789", 0);
    if (pos != string::npos)
    {
        type = val.substr(pos, val.length());
        numStr = val.substr(0, pos);
    }
    else
        numStr = val.substr(0, val.length());

    iss.str(numStr);
    iss >> num;
    if (iss.fail())
        throw splitStringToTypeFail();
    return make_pair(num, type);
}

time_t convertStringToTime(string val)
{
    int pos;
    time_t numTime;
    string timeType[4] = {"h", "m", "s", "ms"};
    pair<ssize_t, string> valuePair;

    valuePair = splitStringToType(val);
    if (valuePair.second == "")
        valuePair.second = "ms";
    numTime = static_cast<time_t>(valuePair.first);

    for (pos = 0; pos < 4; pos++)
    {
        if (timeType[pos] == valuePair.second)
            break;
    }
    switch (pos)
    {
    case 0:
        if (numTime > 2562047788)
            throw convertStringToTimeFail();
        numTime *= 60;
    case 1:
        if (numTime > 153722867280)
            throw convertStringToTimeFail();
        numTime *= 60;
    case 2:
        if (numTime > 9223372036854)
            throw convertStringToTimeFail();
        numTime *= 1000;
    case 3:
        break;
    case 4:
        throw convertStringToTimeFail();
    }
    return numTime;
}

ssize_t convertStringToByte(string val)
{
    char type;
    ssize_t numByte;
    pair<ssize_t, string> valuePair;

    valuePair = splitStringToType(val);
    if (valuePair.second.length() > 1 || valuePair.second.find_first_not_of("gmkGMK", 0) != string::npos)
        throw convertStringToByteFail();
    numByte = valuePair.first;

    type = *(valuePair.second.c_str());
    switch (tolower(type))
    {
    case 'g':
        if (numByte > 8589934591)
            throw convertStringToByteFail();
        numByte *= 1024;
    case 'm':
        if (numByte > 8796093022207)
            throw convertStringToByteFail();
        numByte *= 1024;
    case 'k':
        if (numByte > 9007199254740991)
            throw convertStringToByteFail();
        numByte *= 1024;
        break;
    }
    return numByte;
}
//=====================================parse_util========================================
//=====================================parse_arg========================================

void parse_root(vector<string> arg, Config *config)
{
    if (arg.size() != 1 || config->dupeCheck.root == true)
        throw Config::parseRootFail();
    isPath(arg[0]);
    config->root = arg[0];
    config->dupeCheck.root = true;

    // cout << typeid(config).name() << endl;
    // // HttpConfig *http = dynamic_cast<HttpConfig *>(config);
    // ServerConfig *server = dynamic_cast<ServerConfig *>(config);
    // // LocationConfig *location = dynamic_cast<LocationConfig *>(config);
}
void parse_file_access(vector<string> arg, Config *config)
{
    string lowString;
    LocationConfig *location;
    if (arg.size() != 1)
        throw Config::parseCDflagFail();
    location = dynamic_cast<LocationConfig *>(config);
    if (location == NULL)
        throw Config::parseCDflagFail("file_access directive must use location block");
    lowString = convertStringToLower(arg[0]);
    if (lowString == "off")
        location->file_access = false;
    else if (lowString == "on")
        location->file_access = true;
    else
        throw Config::parseCDflagFail();
}

void parse_alias(vector<string> arg, Config *config)
{
    LocationConfig *location;

    if (arg.size() != 1 || config->dupeCheck.root == true)
        throw Config::parseAliasFail();
    isPath(arg[0]);
    location = dynamic_cast<LocationConfig *>(config);
    if (location == NULL)
        throw Config::parseAliasFail("alias directive must use location block");
    location->alias = arg[0];
    config->dupeCheck.root = true;
}

void parse_listen(vector<string> arg, Config *config)
{

    string::size_type pos;
    unsigned short port;
    string ip = "";

    if (arg.size() != 1)
        throw Config::parseListenFail();
    ServerConfig *server = dynamic_cast<ServerConfig *>(config);
    if (server == NULL)
        throw Config::parseListenFail("listen directive must use server block");
    if ((pos = arg[0].find(':')) != string::npos)
    {
        ip = convertStringToIP(arg[0].substr(0, pos));
        port = convertStringToPort(arg[0].substr(pos + 1, arg[0].length()));
    }
    else
    {
        port = convertStringToPort(arg[0]);
        ip = "127.0.0.1";
    }
    // server->port.push_back(convertStringToPort(arg[0]));

    for (vector<_Addr>::size_type i = 0; i < server->ipPort.size(); i++)
    {
        if (server->ipPort[i].first == ip && server->ipPort[i].second == port)
            throw Config::parseListenFail();
    }
    server->ipPort.push_back(make_pair(ip, port));
    // server->ip.push_back(ip);
    // server->port.push_back(port);
}

void parse_server_names(vector<string> arg, Config *config)
{
    ServerConfig *server = dynamic_cast<ServerConfig *>(config);
    if (server == NULL)
        throw Config::parseServerNameFail("server_names directive must use server block");
    for (vector<string>::size_type i = 0; i < arg.size(); i++)
        server->server_names.push_back(arg[i]);
}

void parse_index(vector<string> arg, Config *config)
{
    if (!config->handdownIndex)
    {
        config->index.clear();
        config->handdownIndex = true;
    }
    for (vector<string>::size_type i = 0; i < arg.size(); i++)
    {
        isPath(arg[i]);
        config->index.push_back(arg[i]);
    }
    config->dupeCheck.index = true;
}

void parse_auto_index(vector<string> arg, Config *config)
{

    string lowString;

    if (arg.size() != 1 || config->dupeCheck.autoindex == true)
        throw Config::parseAutoIndexFail();
    lowString = convertStringToLower(arg[0]);
    if (lowString == "off")
        config->auto_index = false;
    else if (lowString == "on")
        config->auto_index = true;
    else
        throw Config::parseAutoIndexFail();
    config->dupeCheck.autoindex = true;
}

bool exsitErrorpageEqual(string arg, int &equalstatus)
{
    size_t arglen = arg.size();

    if (arglen == 1 && arg[0] == '=')
    {
        equalstatus = 200;
        return true;
    }
    else if (arg[0] == '=')
    {
        arg = arg.substr(1);

        std::stringstream ssInt(arg);

        ssInt >> equalstatus;
        if (ssInt.fail())
            return false;
        return true;
    }
    else
        return false;
}

void parse_error_page(vector<string> arg, Config *config)
{
    string path;
    int status;
    int equalstatus;

    if (arg.size() < 2)
        throw Config::parseErrorPageFail("ErrorPage : arg size fail");
    if (!config->handdownErrorPage)
    {
        config->error_page.clear();
        config->handdownErrorPage = true;
    }
    isPath(arg[arg.size() - 1]);
    path = arg[arg.size() - 1];
    if (exsitErrorpageEqual(arg[arg.size() - 2], equalstatus))
    {
        for (vector<string>::size_type i = 0; i < (arg.size() - 2); i++)
        {
            status = convertStringToStateCode(arg[i]);
            config->error_page[status] = make_pair(equalstatus, path);
        }
    }
    else
    {
        for (vector<string>::size_type i = 0; i < (arg.size() - 1); i++)
        {
            status = convertStringToStateCode(arg[i]);
            config->error_page[status] = make_pair(status, path);
        }
    }
}

void parse_cgi(vector<string> arg, Config *config)
{
    if (arg.size() != 2)
        throw Config::parseCgiFail("parseCgiFail : size");
    if (arg[0][0] != '.')
        throw Config::parseCgiFail("parseCgiFail : dot");
    config->cgi[arg[0]] = arg[1];
}

void parse_keepalive_requests(vector<string> arg, Config *config)
{
    if (arg.size() != 1 || config->dupeCheck.keepalive_requests == true)
        throw Config::parseKeepRequestsFail();
    config->keepalive_requests = convertStringToSsize_T(arg[0]);
    config->dupeCheck.keepalive_requests = true;
}

void parse_default_type(vector<string> arg, Config *config)
{
    if (arg.size() != 1 || config->dupeCheck.default_type == true)
        throw Config::parseDefaultTypeFail();
    config->default_type = arg[0];
    config->dupeCheck.default_type = true;
}

void parse_client_max_body_size(vector<string> arg, Config *config)
{
    if (arg.size() != 1 || config->dupeCheck.client_max_body_size == true)
        throw Config::parseClientBodySizeFail();
    config->client_max_body_size = convertStringToByte(arg[0]);
    config->dupeCheck.client_max_body_size = true;
}

void parse_reset_timedout_connection(vector<string> arg, Config *config)
{
    string button;

    if (arg.size() != 1 || config->dupeCheck.reset_timedout_connection == true)
        throw Config::parseResetTimedoutConnFail();
    button = convertStringToLower(arg[0]);
    if (button == "off")
        config->reset_timedout_connection = 0;
    else
        config->reset_timedout_connection = 1;
    config->dupeCheck.reset_timedout_connection = true;
}

void parse_lingering_timeout(vector<string> arg, Config *config)
{
    if (arg.size() != 1 || config->dupeCheck.lingering_timeout == true)
        throw Config::parseLingeringTimeoutFail();
    config->lingering_timeout = convertStringToTime(arg[0]);
    config->dupeCheck.lingering_timeout = true;
}

void parse_lingering_time(vector<string> arg, Config *config)
{
    if (arg.size() != 1 || config->dupeCheck.lingering_time == true)
        throw Config::parseLingeringTimeFail();
    config->lingering_time = convertStringToTime(arg[0]);
    config->dupeCheck.lingering_time = true;
}

void parse_keepalive_time(vector<string> arg, Config *config)
{
    if (arg.size() != 1 || config->dupeCheck.keepalive_time == true)
        throw Config::parseKeepTimeFail();
    config->keepalive_time = convertStringToTime(arg[0]);
    config->dupeCheck.keepalive_time = true;
}

void parse_keepalive_timeout(vector<string> arg, Config *config)
{
    if (arg.size() != 1 || config->dupeCheck.keepalive_timeout == true)
        throw Config::parseKeepTimeoutFail();
    config->keepalive_timeout = convertStringToTime(arg[0]);
    config->dupeCheck.keepalive_timeout = true;
}

void parse_send_timeout(vector<string> arg, Config *config)
{
    if (arg.size() != 1 || config->dupeCheck.send_timeout == true)
        throw Config::parseSendTimeoutFail();
    config->send_timeout = convertStringToTime(arg[0]);
    config->dupeCheck.send_timeout = true;
}

void parse_client_body_timeout(vector<string> arg, Config *config)
{
    if (arg.size() != 1 || config->dupeCheck.client_body_timeout == true)
        throw Config::parseClientBodyTimeoutFail();
    config->client_body_timeout = convertStringToTime(arg[0]);
    config->dupeCheck.client_body_timeout = true;
}

void parse_limit_except_method(vector<string> arg, Config *config)
{
    LocationConfig *location;
    string method;
    vector<string>::size_type check;

    location = dynamic_cast<LocationConfig *>(config);
    if (location == NULL)
        throw parseLimitExceptMethodFail("limit_except_method directive must use location block");
    location->checkSetLimitExceptMethod = true;
    for (vector<string>::size_type i = 0; i < arg.size(); i++)
    {
        method = convertStringToUpper(arg[i]);
        check = 0;
        for (check = 0; check < location->limit_except_method.size(); check++)
        {
            if (location->limit_except_method[check] == method)
                break;
        }
        if (check == location->limit_except_method.size() && (method == "GET" || method == "POST" || method == "PUT" || method == "DELETE"))
            location->limit_except_method.push_back(method);
        else
            throw parseLimitExceptMethodFail();
    }
}

//=====================================parse_arg========================================

void EraseComment(string &configtemp)
{
    size_t start = 0;
    size_t end = 0;

    while ((start = configtemp.find('#')) != string::npos)
    {
        end = configtemp.find('\n', start);
        configtemp.erase(start, end - start);
    }
}

string ReadConfig(char **argv)
{
    string line, configtemp;
    ifstream file(*(argv + 1));

    if (file.is_open())
    {
        while (getline(file, line))
            if (line.length() != 0)
                configtemp += line + '\n';
        file.close();
    }
    else
        cout << "Unable to open file"; // throw

    EraseComment(configtemp);
    return (configtemp);
}

//-----------------print config-----------------

// void printConfig(Config *link)
// {
//     cout << "index : ";
//     for (vector<string>::size_type i = 0; i < link->index.size(); i++)
//     {
//         cout << link->index[i] << " ";
//     }
//     cout << endl;

//     cout << "root : " << link->root << endl;

//     cout << "auto_index : " << link->auto_index << endl;

//     cout << "error_page :" << endl;
//     ;
//     for (map<int, string>::iterator iter = link->error_page.begin(); iter != link->error_page.end(); iter++)
//     {
//         cout << "key : " << iter->first << "value : " << iter->second << endl;
//     }
//     cout << "keepalive_requests : " << link->keepalive_requests << endl;

//     cout << "default_type : " << link->default_type << endl;

//     cout << "client_max_body_size : " << link->client_max_body_size << endl;

//     cout << "reset_timedout_connection : " << link->reset_timedout_connection << endl;

//     cout << "lingering_timeout : " << link->lingering_timeout << endl;

//     cout << "lingering_time : " << link->lingering_time << endl;

//     cout << "keepalive_time : " << link->keepalive_time << endl;

//     cout << "keepalive_timeout : " << link->keepalive_timeout << endl;

//     cout << "send_timeout : " << link->send_timeout << endl;

//     cout << "client_body_timeout : " << link->client_body_timeout << endl;
// }

// void pringConfigAll(Config *link)
// {
//     LocationConfig *location;
//     ServerConfig *server;

//     cout << "=============== http ===============" << endl;
//     printConfig(link);
//     for (vector<Config *>::size_type i = 0; i < link->link.size(); i++)
//     {
//         cout << "=============== server " << i << "===============" << endl;
//         server = dynamic_cast<ServerConfig *>(link->link[i]);
//         for (vector<_Addr>::size_type j = 0; j < server->ipPort.size(); j++)
//         {
//             cout << "ip : " << server->ipPort[j].first << " port : " << server->ipPort[j].second << endl;
//         }
//         for (vector<string>::size_type j = 0; j < server->server_names.size(); j++)
//         {
//             cout << "serve_rname : " << server->server_names[j] << endl;
//         }
//         printConfig(link->link[i]);
//         for (vector<Config *>::size_type j = 0; j < link->link[i]->link.size(); j++)
//         {
//             cout << "=============== location " << j << "===============" << endl;
//             location = dynamic_cast<LocationConfig *>(link->link[i]->link[j]);
//             cout << "URI : " << location->URI << endl;
//             cout << "= : " << location->assign << endl;
//             for (vector<string>::size_type k = 0; k < location->limit_except_method.size(); k++)
//                 cout << "limit_except_method : " << location->limit_except_method[k] << endl;
//             printConfig(link->link[i]->link[j]);
//             cout << "=============== location " << j << "===============" << endl;
//         }
//         cout << "=============== server " << i << " ===============" << endl;
//     }
//     cout << "=============== http ===============" << endl;
// }
