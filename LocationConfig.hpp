
#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

// #include "Config.hpp"

class Config;

class LocationConfig : public Config
{
    /**========================================================================
     * %                          member variables
     *========================================================================**/

    string URI;
    // bool assign;
    vector<string> limit_except_method;

    /**========================================================================
     * @                           Constructors
     *========================================================================**/

public:
    LocationConfig() : Config() {}
    LocationConfig(string str, string uri) : Config()
    {
        configtemp = str;
        URI = uri;
        cout << "loc" << endl;
        SetupConfig();
        cout << "loc" << endl;
    }
    // LocationConfig(const LocationConfig &src) : Config() {}
    virtual ~LocationConfig() {}

    /**========================================================================
     * *                            operators
     *========================================================================**/

    LocationConfig &operator=(const Config &src)
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
    /**========================================================================
     * !                            Exceptions
     *========================================================================**/
};

#endif