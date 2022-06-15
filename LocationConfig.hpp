
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
        // limit_except_method[0] = "";
        URI = uri;
        assign = checkAssign(URI);
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
    bool checkAssign(string &uri)
    {
        size_t pos;

        if (uri.length() == 0)
            throw Config::parseLocationFail();
        if ((pos = uri.find("=")) != string::npos)
        {
            uri.erase(pos, 1);
            return true;
        }
        return false;
    }

    /**========================================================================
     * !                            Exceptions
     *========================================================================**/
};

#endif