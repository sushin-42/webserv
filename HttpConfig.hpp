#ifndef HTTPCONFIG_HPP
#define HTTPCONFIG_HPP
#include "ServerConfig.hpp"

class Config;

class HttpConfig : public Config
{
    /**========================================================================
     * @                           Constructors
     *========================================================================**/

public:
    string etc; // http 블록 외에 있는 디렉티브 모음

    HttpConfig() : Config() {}
    virtual ~HttpConfig() {}

    /**========================================================================
     * *                            operators
     *========================================================================**/

    HttpConfig &operator=(const Config &src)
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
    void setConfig(string config)
    {
        etc = config;
        separateHttpBlock();
        cout << etc << endl;
        SeparateServerBlock();
        cout << RED(" http block ") << endl;
        SetupConfig();
        cout << RED(" http block ") << endl;
    }
    void separateHttpBlock()
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
    void SeparateServerBlock()
    {
        size_t start = 0;
        size_t end = 0;
        string serverConfigtemp;

        while ((start = configtemp.find("server ", end)) != string::npos)
        {
            serverConfigtemp = ExtractBlock(configtemp, start);
            Config *servConf = new ServerConfig(serverConfigtemp);
            this->link.push_back(servConf);
        }
    }

    /**========================================================================
     * !                            Exceptions
     *========================================================================**/
};

//*--------------------------------------------------------------------------*//

#endif