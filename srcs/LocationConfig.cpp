#include "LocationConfig.hpp"
#include "ServerConfig.hpp"

LocationConfig::LocationConfig() : Config() {}
LocationConfig::LocationConfig(pair<string, string> locationConfUri, ServerConfig *serverConf) : Config()
{
    setServerDirective(serverConf);
    URI = locationConfUri.first;
    configtemp = locationConfUri.second;
    alias = "";

    assign = checkAssign(URI);
    SetupConfig();
    setLimitExceptMethod();
}
LocationConfig::~LocationConfig() {}
/**========================================================================
 * *                            operators
 *========================================================================**/

LocationConfig &LocationConfig::operator=(const Config &src)
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
bool LocationConfig::checkAssign(string &uri)
{
    size_t pos;

    if (uri.length() == 0)
        throw Config::parseLocationFail();
    if ((pos = uri.find("=")) != string::npos)
    {
        uri.erase(pos, 1);
        uri = trim(uri, " ");
        return true;
    }
    return false;
}

void LocationConfig::setServerDirective(ServerConfig *serverConf)
{
    this->index = serverConf->index;
    this->auto_index = serverConf->auto_index;
    this->root = serverConf->root;
    this->keepalive_requests = serverConf->keepalive_requests;
    this->default_type = serverConf->default_type;
    this->client_max_body_size = serverConf->client_max_body_size;
    this->reset_timedout_connection = serverConf->reset_timedout_connection;
    this->lingering_time = serverConf->lingering_time;
    this->lingering_timeout = serverConf->lingering_timeout;
    this->keepalive_time = serverConf->keepalive_time;
    this->keepalive_timeout = serverConf->keepalive_timeout;
    this->send_timeout = serverConf->send_timeout;
    this->client_body_timeout = serverConf->client_body_timeout;
    this->error_page = serverConf->error_page;
    this->cgi = serverConf->cgi;
    this->timer = serverConf->timer;
    this->file_access = serverConf->file_access;
    // only server directive
    this->server_names = serverConf->server_names;
    this->ipPort = serverConf->ipPort;
    this->d_return = serverConf->d_return;

    this->server_name_in_redirect = serverConf->server_name_in_redirect;
    this->port_in_redirect = serverConf->port_in_redirect;
}
void LocationConfig::setLimitExceptMethod()
{
    if (!this->checkSetLimitExceptMethod)
    {
        this->limit_except_method.push_back("GET");
        this->limit_except_method.push_back("POST");
    }
}

/**========================================================================
 * !                            Exceptions
 *========================================================================**/
