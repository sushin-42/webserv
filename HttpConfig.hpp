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
    HttpConfig() : Config() {}
    HttpConfig(char **argv) : Config()
    {
        ReadConfig(argv);
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

    void ReadConfig(char **argv)
    {
        string line;
        ifstream file(*(argv + 1));

        if (file.is_open())
        {
            while (getline(file, line))
                if (line.length() != 0)
                    configtemp += line + '\n';
            file.close();
        }
        else
            cout << "Unable to open file";

        EraseComment(configtemp);
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