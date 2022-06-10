#ifndef CONFIGUTILS_HPP
#define CONFIGUTILS_HPP
void parse_root(vector<string> arg)
{
    for (size_t i = 0; i < arg.size(); i++)
        cout << arg[i] << " ";
    cout << endl;
    cout << "root success" << endl;
}

void parse_listen(vector<string> arg)
{
    for (size_t i = 0; i < arg.size(); i++)
        cout << arg[i] << " ";
    cout << endl;
    cout << "listen success" << endl;
}

void parse_server_name(vector<string> arg)
{
    for (size_t i = 0; i < arg.size(); i++)
        cout << arg[i] << " ";
    cout << endl;
    cout << "server_name success" << endl;
}

void parse_index(vector<string> arg)
{
    for (size_t i = 0; i < arg.size(); i++)
        cout << arg[i] << " ";
    cout << endl;
    cout << "index success" << endl;
}

void EraseComment(string &configtemp)
{
    std::string::size_type start = 0;
    std::string::size_type end = 0;

    for (size_t i = 0; i < configtemp.length(); i++)
    {
        if (configtemp[i] == '#')
        {
            start = i;
            end = i;
            while (configtemp[end] != '\n')
                end++;
            configtemp.erase(start, end - start);
        }
    }
}
#endif