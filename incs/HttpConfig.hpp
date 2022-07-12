#ifndef HTTPCONFIG_HPP
#define HTTPCONFIG_HPP
#include <iostream>
#include "Config.hpp"

class HttpConfig : public Config
{
    /**========================================================================
     * @                           Constructors
     *========================================================================**/

public:
    string etc; // http 블록 외에 있는 디렉티브 모음
    vector<string> serverConfigtemp;
    _Map serverMap;
    virtual ~HttpConfig();

private:
    HttpConfig(){};
    static HttpConfig *http;

public:
    static HttpConfig *getInstance();

    /**========================================================================
     * *                            operators
     *========================================================================**/

    HttpConfig &operator=(const Config &src);

    /**========================================================================
     * #                          member functions
     *========================================================================**/
    void defaultSet();
    void setConfig(string config);
    void separateHttpBlock();
    void SeparateServerBlock();
    void makeServerBlock();
    void makeServerMap();
    /**========================================================================
     * !                            Exceptions
     *========================================================================**/
};

//*--------------------------------------------------------------------------*//

#endif
