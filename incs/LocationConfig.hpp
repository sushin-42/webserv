
#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP
#include "Config.hpp"
class ServerConfig;

class LocationConfig : public Config
{
    /**========================================================================
     * %                          member variables
     *========================================================================**/
public:
    string URI;  // location URI(/admin/)  {}
    bool assign; //
    vector<string> limit_except_method;

    // server directive
    vector<string> server_name;
    vector<_Addr> ipPort;

    /**========================================================================
     * @                           Constructors
     *========================================================================**/

public:
    LocationConfig();
    LocationConfig(pair<string, string> locationConfUri, ServerConfig *serverConf);
    // LocationConfig(const LocationConfig &src) : Config() {}
    virtual ~LocationConfig();
    /**========================================================================
     * *                            operators
     *========================================================================**/

    LocationConfig &operator=(const Config &src);

    /**========================================================================
     * #                          member functions
     *========================================================================**/
    bool checkAssign(string &uri);

    void setServerDirective(ServerConfig *serverConf);
    /**========================================================================
     * !                            Exceptions
     *========================================================================**/
};

#endif