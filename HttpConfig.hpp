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
    string etc;

    HttpConfig() : Config() {}
    HttpConfig(string config) : Config()
    {
        etc = config;
        separateHttpBlock();
        cout << etc << endl;
        SeparateServerBlock();
        cout << RED(" http block ") << endl;
        SetupConfig();
        cout << RED(" http block ") << endl;
    }
    // HttpConfig(const HttpConfig &src) : Config() {}
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

    void separateHttpBlock()
    {
        size_t start = 0;
        size_t end = 0;

        while ((start = etc.find("http ", end)) != string::npos)
        {
            if (configtemp.empty())
                configtemp = ExtractBlock(etc, start);
            else
                return; // duplicate 에러 내뱉기
        }
        if (configtemp.empty())
            return; // http 블록이 없습니다 에러 내뱉기
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