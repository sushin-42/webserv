
#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP
#include "Config.hpp"
class Config;

class LocationConfig : public Config
{
    /**========================================================================
     * %                          member variables
     *========================================================================**/
public:
    string URI;  // location URI(/admin/)  {}
    bool assign; //
    vector<string> limit_except_method;

    /**========================================================================
     * @                           Constructors
     *========================================================================**/

public:
    LocationConfig() : Config() {}
    LocationConfig(string str, const string uri) : Config()
    {
        configtemp = str;
        URI = uri;
        assign = checkAssign(uri);
        cout << BLUE(" location block ") << endl;
        SetupConfig();
        cout << BLUE(" location block ") << endl;
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
    bool checkAssign(const string uri)
    {
        if (uri.find("=") != string::npos)
            return true;
        return false;
    }

    /**========================================================================
     * !                            Exceptions
     *========================================================================**/
};

#endif