#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP
#include "Config.hpp"
class ServerConfig : public Config
{
    /**========================================================================
     * %                          member variables
     *========================================================================**/
private:
    typedef pair<string, string> _LocUri;

public:
    vector<_LocUri> locationConfUri;
    vector<string> server_name;
    vector<_Addr> ipPort;

    /**========================================================================
     * @                           Constructors
     *========================================================================**/

public:
    ServerConfig();
    ServerConfig(string str, Config *httpConf);
    // ServerConfig(const ServerConfig &src) : Config() {}
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
    /**========================================================================
     * !                            Exceptions
     *========================================================================**/
};

//*--------------------------------------------------------------------------*//

#endif
