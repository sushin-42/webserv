
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
    bool assign;
    vector<string> limit_except_method;
    bool checkSetLimitExceptMethod;
    string alias;
    

    // server directive
    vector<string> server_names;
    vector<_Addr> ipPort;

    /**========================================================================
     * @                           Constructors
     *========================================================================**/

public:
    LocationConfig();
    LocationConfig(_LocUri locationConfUri, ServerConfig *serverConf);
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
    void setLimitExceptMethod();
    /**========================================================================
     * !                            Exceptions
     *========================================================================**/
};

#endif
