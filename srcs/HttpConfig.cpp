
#include "HttpConfig.hpp"
#include "ServerConfig.hpp"
#include "ConfigUtils.hpp"
HttpConfig *HttpConfig::http;
class Config;
HttpConfig::~HttpConfig() {}

/**========================================================================
 * *                            operators
 *========================================================================**/

HttpConfig &HttpConfig::operator=(const Config &src)
{
    if (this != &src)
    {
        this->Config::operator=(src);
    }
    return *this;
}

/**========================================================================
 * #                          member functions
 *========================================================================**/
HttpConfig *HttpConfig::getInstance()
{

    if (http == NULL)
    {
        http = new HttpConfig();
    }
    return http;
}
void HttpConfig::defaultSet()
{
    for (size_t i = 0; i < sizeof(dupeCheck); i++)
    {
        if (dupeCheck[i] == false)
        {
            switch (i)
            {
            case 0:
                index.push_back("index.html");
                break;
            case 1:
                root = "html";
                break;
            case 2:
                auto_index = false;
                break;
            case 3:
                keepalive_time = convertStringToTime("1h");
                break;
            case 4:
                keepalive_timeout = convertStringToTime("75s");
                break;
            case 5:
                keepalive_requests = 100;
                break;
            case 6:
                default_type = "text/plain";
                break;
            case 7:
                client_max_body_size = convertStringToByte("1m");
                break;
            case 8:
                reset_timedout_connection = false;
                break;
            case 9:
                lingering_time = convertStringToTime("30s");
                break;
            case 10:
                lingering_timeout = convertStringToTime("5s");
                break;
            case 11:
                send_timeout = convertStringToTime("60s");
                break;
            case 12:
                client_body_timeout = convertStringToTime("60s");
                break;
            case 13:
                timer = convertStringToTime("20s");
                break;
            case 14:
	            server_name_in_redirect = false;
                break;
            case 15:
	            port_in_redirect = true;
                break;
            default:
                break;
            }
        }
    }
}
void HttpConfig::setConfig(string config)
{
    
    etc = config;
    defaultSet();
    separateHttpBlock();
    SeparateServerBlock();
    SetupConfig();
    makeServerBlock();
    makeServerMap();
    
}

void HttpConfig::makeServerMap()
{
    for (size_t k = 0; k < link.size(); k++)
    {
        ServerConfig *serv = CONVERT(link[k], ServerConfig);

        for (size_t i = 0; i < serv->ipPort.size(); i++)
            serverMap[serv->ipPort[i]].push_back(link[k]);
    }
}

void HttpConfig::separateHttpBlock()
{
    size_t start = 0;
    size_t end = 0;

    while ((start = etc.find("http ", end)) != string::npos)
    {
        if (configtemp.empty())
            configtemp = ExtractBlock(etc, start);
        else
            throw Config::httpDupe();
    }
    if (configtemp.empty())
        throw Config::notExistHttpBlock();
}
void HttpConfig::SeparateServerBlock()
{
    size_t start = 0;
    size_t end = 0;

    while ((start = configtemp.find("server ", end)) != string::npos)
        serverConfigtemp.push_back(ExtractBlock(configtemp, start));
}
void HttpConfig::makeServerBlock()
{
    for (size_t i = 0; i < serverConfigtemp.size(); i++)
    {
        Config *servConf = new ServerConfig(serverConfigtemp[i], this);
        this->link.push_back(servConf);
    }
}

/**========================================================================
 * !                            Exceptions
 *========================================================================**/

//*--------------------------------------------------------------------------*//
