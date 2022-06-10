#ifndef CONFIGUTILS_HPP
#define CONFIGUTILS_HPP
void parse_root(vector<string> arg)
{
    for (size_t i = 0; i < arg.size(); i++)
        cout << arg[i] << " ";
    cout << endl;
    cout << GREEN("root success") << endl;
}

void parse_listen(vector<string> arg)
{
    for (size_t i = 0; i < arg.size(); i++)
        cout << arg[i] << " ";
    cout << endl;
    cout << GREEN("listen success") << endl;
}

void parse_server_name(vector<string> arg)
{
    for (size_t i = 0; i < arg.size(); i++)
        cout << arg[i] << " ";
    cout << endl;
    cout << GREEN("server_name success") << endl;
}

void parse_index(vector<string> arg)
{
    for (size_t i = 0; i < arg.size(); i++)
        cout << arg[i] << " ";
    cout << endl;
    cout << GREEN("index success") << endl;
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
#endif