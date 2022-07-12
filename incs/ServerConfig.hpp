#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP
#include "Config.hpp"
class ServerConfig : public Config
{
    /**========================================================================
     * %                          member variables
     *========================================================================**/

public:
    vector<_LocUri> locationConfUri;
    vector<string> server_names;
    vector<_Addr> ipPort;

    /**========================================================================
     * @                           Constructors
     *========================================================================**/

public:
    ServerConfig();
    ServerConfig(string str, Config *httpConf);
    virtual ~ServerConfig();

    /**========================================================================
     * *                            operators
     *========================================================================**/

    ServerConfig &operator=(const ServerConfig &src);
    /**========================================================================
     * #                          member functions
     *========================================================================**/
    string ExtractURI(size_t start);

    void SeparateLocationBlock();
    void makeLocationBlock();
    void setHttpDirective(Config *httpConf);
    void checkServername();
    /**========================================================================
     * !                            Exceptions
     *========================================================================**/
};

//*--------------------------------------------------------------------------*//

#endif
