#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP
#include "LocationConfig.hpp"
class Config;

class ServerConfig : public Config
{
    /**========================================================================
     * %                          member variables
     *========================================================================**/

private:
    vector<string> server_name;
    vector<string> ip;
    // unsigned short port;

    /**========================================================================
     * @                           Constructors
     *========================================================================**/

public:
    ServerConfig() : Config() {}
    ServerConfig(string str) : Config()
    {
        configtemp = str;
        SeparateLocationBlock();
        cout << PURPLE(" server block ") << endl;
        SetupConfig();
        cout << PURPLE(" server block ") << endl;
    }
    // ServerConfig(const ServerConfig &src) : Config() {}
    virtual ~ServerConfig() {}

    /**========================================================================
     * *                            operators
     *========================================================================**/

    ServerConfig &operator=(const ServerConfig &src)
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
    string ExtractURI(size_t start)
    {
        size_t end;
        string uri;

        end = configtemp.find('{', start);
        uri = configtemp.substr(start, end - start);
        return (uri);
    }

    void SeparateLocationBlock()
    {
        size_t start = 0;
        size_t end = 0;
        string locationConfigtemp;

        while ((start = configtemp.find("location ", end)) != string::npos)
        {
            string uri = ExtractURI(start + 8);
            locationConfigtemp = ExtractBlock(configtemp, start);
            Config *locConf = new LocationConfig(locationConfigtemp, uri);
            this->link.push_back(locConf);
        }
    }
    /**========================================================================
     * !                            Exceptions
     *========================================================================**/
};

//*--------------------------------------------------------------------------*//

#endif