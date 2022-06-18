#include "ServerConfig.hpp"
// #include "Config.hpp"
#include "LocationConfig.hpp"

class LocationConfig;

/**========================================================================
 * @                           Constructors
 *========================================================================**/

ServerConfig::ServerConfig() : Config() {}
ServerConfig::ServerConfig(string str, Config *httpConf) : Config()
{
    cout << PURPLE(" server block ") << endl;
    setHttpDirective(httpConf);
    // impl set serverconfig variables
    configtemp = str;
    SeparateLocationBlock();
    SetupConfig();
    makeLocationBlock();
    cout << PURPLE(" server block ") << endl;
}
// ServerConfig(const ServerConfig &src) : Config() {}
ServerConfig::~ServerConfig() {}

/**========================================================================
 * *                            operators
 *========================================================================**/

ServerConfig &ServerConfig::operator=(const ServerConfig &src)
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
string ServerConfig::ExtractURI(size_t start)
{
    size_t end;
    string uri;

    end = configtemp.find('{', start);
    uri = configtemp.substr(start, end - start);
    uri = trim(uri, " ");
    return (uri);
}

void ServerConfig::SeparateLocationBlock()
{
    size_t start = 0;
    size_t end = 0;
    string locationConfigtemp;
    string uri;
    while ((start = configtemp.find("location ", end)) != string::npos)
    {
        uri = ExtractURI(start + 8);
        locationConfigtemp = ExtractBlock(configtemp, start);
        locationConfUri.push_back(make_pair(uri, locationConfigtemp));
    }
}
void ServerConfig::makeLocationBlock()
{
    for (size_t i = 0; i < locationConfUri.size(); i++)
    {
        Config *locConf = new LocationConfig(locationConfUri[i], this);
        this->link.push_back(locConf);
    }
}
void ServerConfig::setHttpDirective(Config *httpConf)
{
    this->index = httpConf->index;
    this->auto_index = httpConf->auto_index;
    this->root = httpConf->root;
    this->keepalive_requests = httpConf->keepalive_requests;
    this->default_type = httpConf->default_type;
    this->client_max_body_size = httpConf->client_max_body_size;
    this->reset_timedout_connection = httpConf->reset_timedout_connection;
    this->lingering_time = httpConf->lingering_time;
    this->lingering_timeout = httpConf->lingering_timeout;
    this->keepalive_time = httpConf->keepalive_time;
    this->keepalive_timeout = httpConf->keepalive_timeout;
    this->send_timeout = httpConf->send_timeout;
    this->client_body_timeout = httpConf->client_body_timeout;
}
/**========================================================================
 * !                            Exceptions
 *========================================================================**/

//*--------------------------------------------------------------------------*//
