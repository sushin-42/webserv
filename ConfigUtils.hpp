#ifndef CONFIGUTILS_HPP
#define CONFIGUTILS_HPP
#include "Config.hpp"
#include "ServerConfig.hpp"
// class Config;
class HttpConfig;
class ServerConfig;
class LocationConfig;
void parse_root(vector<string> arg, Config *config)
{
    for (size_t i = 0; i < arg.size(); i++)
        cout << arg[i] << " ";
    cout << endl;
    cout << GREEN("root success") << endl;
    cout << GREEN("") << endl;
    cout << typeid(config).name() << endl;
    // HttpConfig *http = dynamic_cast<HttpConfig *>(config);
    ServerConfig *server = dynamic_cast<ServerConfig *>(config);
    // LocationConfig *location = dynamic_cast<LocationConfig *>(config);

    // cout << typeid(http).name() << endl;
    cout << typeid(server).name() << endl;
    // cout << typeid(location).name() << endl;

    cout << GREEN("") << endl;
    // throw Config::parseFail();
}

void parse_listen(vector<string> arg, Config *config)
{
    for (size_t i = 0; i < arg.size(); i++)
        cout << arg[i] << " ";
    cout << endl;
    cout << GREEN("listen success") << endl;
    cout << typeid(config).name() << endl;
}

void parse_server_name(vector<string> arg, Config *config)
{
    for (size_t i = 0; i < arg.size(); i++)
        cout << arg[i] << " ";
    cout << endl;
    cout << GREEN("server_name success") << endl;
    cout << typeid(config).name() << endl;
}

void parse_index(vector<string> arg, Config *config)
{
    for (size_t i = 0; i < arg.size(); i++)
        cout << arg[i] << " ";
    cout << endl;
    cout << GREEN("index success") << endl;
    cout << typeid(config).name() << endl;
}

void EraseComment(string &configtemp)
{
    size_t start = 0;
    size_t end = 0;

    while ((start = configtemp.find('#')) != string::npos)
    {
        end = configtemp.find('\n', start);
        configtemp.erase(start, end - start);
    }
}

string ReadConfig(char **argv)
{
    string line, configtemp;
    ifstream file(*(argv + 1));

    if (file.is_open())
    {
        while (getline(file, line))
            if (line.length() != 0)
                configtemp += line + '\n';
        file.close();
    }
    else
        cout << "Unable to open file"; // throw

    EraseComment(configtemp);
    return (configtemp);
}
#endif