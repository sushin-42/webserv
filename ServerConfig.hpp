#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

// #include "Config.hpp"

class Config;

class ServerConfig : public Config
{
    /**========================================================================
     * %                          member variables
     *========================================================================**/

private:
    string server_name;
    string ip;
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
        cout << "ser" << endl;
        SetupConfig();
        cout << "ser" << endl;
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
    string ExtractURI(string::size_type start)
    {
        string::size_type end;
        string uri;

        end = configtemp.find('{', start);
        uri = configtemp.substr(start, end - start);
        return (uri);
    }

    void SeparateLocationBlock()
    {
        std::string::size_type start = 0;
        std::string::size_type end = 0;
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