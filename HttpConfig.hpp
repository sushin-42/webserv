#ifndef HTTPCONFIG_HPP
#define HTTPCONFIG_HPP



class HttpConfig : public Config
{
    /**========================================================================
     * @                           Constructors
     *========================================================================**/

public:
    string etc; // http 블록 외에 있는 디렉티브 모음
    vector<string> serverConfigtemp;
    map<pair<string, unsigned short>, vector<ServerConfig *> > serverMap;
    virtual ~HttpConfig() {}

private:
    static HttpConfig *http;
    HttpConfig() : Config()
    {
    }

public:
    static HttpConfig *getInstance()
    {

        if (http == NULL)
        {
            http = new HttpConfig();
        }
        return http;
    }

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
    void defaultSet()
    {
        for (size_t i = 0; i < sizeof(dupeCheck); i++)
        {
            if (dupeCheck[i] == false)
            {
                switch (i)
                {
                case 0:
                    index.push_back("index.html");
                    break;
                case 1:
                    root = "html";
                    break;
                case 2:
                    auto_index = false;
                    break;
                case 3:
                    keepalive_time = convertStringToTime("1h");
                    break;
                case 4:
                    keepalive_timeout = convertStringToTime("75s");
                    break;
                case 5:
                    keepalive_requests = 100;
                    break;
                case 6:
                    default_type = "text/plain";
                    break;
                case 7:
                    client_max_body_size = convertStringToByte("1m");
                    break;
                case 8:
                    reset_timedout_connection = false;
                    break;
                case 9:
                    lingering_time = convertStringToTime("30s");
                    break;
                case 10:
                    lingering_timeout = convertStringToTime("5s");
                    break;
                case 11:
                    send_timeout = convertStringToTime("60s");
                    break;
                case 12:
                    client_body_timeout = convertStringToTime("60s");
                    break;
                }
            }
        }
    }
    void setConfig(string config)
    {
        cout << RED(" http block ") << endl;
        etc = config;
        defaultSet();
        separateHttpBlock();
        SeparateServerBlock();
        SetupConfig();
        makeServerBlock();
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

        while ((start = configtemp.find("server ", end)) != string::npos)
            serverConfigtemp.push_back(ExtractBlock(configtemp, start));
    }
    void makeServerBlock()
    {
        for (size_t i = 0; i < serverConfigtemp.size(); i++)
        {
            Config *servConf = new ServerConfig(serverConfigtemp[i], this);
            this->link.push_back(servConf);
        }
    }
   
    /**========================================================================
     * !                            Exceptions
     *========================================================================**/
};
HttpConfig *HttpConfig::http;
//*--------------------------------------------------------------------------*//

#endif
