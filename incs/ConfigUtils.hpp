#ifndef CONFIGUTILS_HPP
#define CONFIGUTILS_HPP

#include "ConfigUtilsException.hpp"
#include <sstream>
#include <fstream>
#include <arpa/inet.h>

using namespace std;

typedef pair<string, unsigned short> _Addr;
// typedef vector<Config *> _Confs;
// typedef map<_Addr, _Confs> _Map;
//=====================================parse_util========================================
void isPath(string path);
string convertStringToLower(string str);

string convertStringToUpper(string str);

ssize_t convertStringToSsize_T(string val);

int convertStringToStateCode(string code);

string convertStringToIpv4(unsigned int ip);
string convertStringToIP(string ip);
unsigned short convertStringToPort(string code);


pair<ssize_t, string> splitStringToType(string val);
time_t convertStringToTime(string val);
ssize_t convertStringToByte(string val);
//=====================================parse_util========================================
//=====================================parse_arg========================================

void parse_root(vector<string> arg, Config *config);
void parse_listen(vector<string> arg, Config *config);
void parse_server_name(vector<string> arg, Config *config);
void parse_index(vector<string> arg, Config *config);
void parse_auto_index(vector<string> arg, Config *config);
void parse_error_page(vector<string> arg, Config *config);
void parse_keepalive_requests(vector<string> arg, Config *config);
void parse_default_type(vector<string> arg, Config *config);
void parse_client_max_body_size(vector<string> arg, Config *config);
void parse_reset_timedout_connection(vector<string> arg, Config *config);
void parse_lingering_timeout(vector<string> arg, Config *config);
void parse_lingering_time(vector<string> arg, Config *config);
void parse_keepalive_time(vector<string> arg, Config *config);
void parse_keepalive_timeout(vector<string> arg, Config *config);

void parse_send_timeout(vector<string> arg, Config *config);
void parse_client_body_timeout(vector<string> arg, Config *config);
void parse_limit_except_method(vector<string> arg, Config *config);
void parse_alias(vector<string> arg, Config *config);

//=====================================parse_arg========================================

void EraseComment(string &configtemp);
string ReadConfig(char **argv);

//-----------------print config-----------------

// void printConfig(Config *link)
// {
//     cout << "index : ";
//     for (vector<string>::size_type i = 0; i < link->index.size(); i++)
//     {
//         cout << link->index[i] << " ";
//     }
//     cout << endl;

//     cout << "root : " << link->root << endl;

//     cout << "auto_index : " << link->auto_index << endl;

//     cout << "error_page :" << endl;
//     ;
//     for (map<int, string>::iterator iter = link->error_page.begin(); iter != link->error_page.end(); iter++)
//     {
//         cout << "key : " << iter->first << "value : " << iter->second << endl;
//     }
//     cout << "keepalive_requests : " << link->keepalive_requests << endl;

//     cout << "default_type : " << link->default_type << endl;

//     cout << "client_max_body_size : " << link->client_max_body_size << endl;

//     cout << "reset_timedout_connection : " << link->reset_timedout_connection << endl;

//     cout << "lingering_timeout : " << link->lingering_timeout << endl;

//     cout << "lingering_time : " << link->lingering_time << endl;

//     cout << "keepalive_time : " << link->keepalive_time << endl;

//     cout << "keepalive_timeout : " << link->keepalive_timeout << endl;

//     cout << "send_timeout : " << link->send_timeout << endl;

//     cout << "client_body_timeout : " << link->client_body_timeout << endl;
// }

// void pringConfigAll(Config *link)
// {
//     LocationConfig *location;
//     ServerConfig *server;

//     cout << "=============== http ===============" << endl;
//     printConfig(link);
//     for (vector<Config *>::size_type i = 0; i < link->link.size(); i++)
//     {
//         cout << "=============== server " << i << "===============" << endl;
//         server = dynamic_cast<ServerConfig *>(link->link[i]);
//         for (vector<_Addr>::size_type j = 0; j < server->ipPort.size(); j++)
//         {
//             cout << "ip : " << server->ipPort[j].first << " port : " << server->ipPort[j].second << endl;
//         }
//         for (vector<string>::size_type j = 0; j < server->server_name.size(); j++)
//         {
//             cout << "serve_rname : " << server->server_name[j] << endl;
//         }
//         printConfig(link->link[i]);
//         for (vector<Config *>::size_type j = 0; j < link->link[i]->link.size(); j++)
//         {
//             cout << "=============== location " << j << "===============" << endl;
//             location = dynamic_cast<LocationConfig *>(link->link[i]->link[j]);
//             cout << "URI : " << location->URI << endl;
//             cout << "= : " << location->assign << endl;
//             for (vector<string>::size_type k = 0; k < location->limit_except_method.size(); k++)
//                 cout << "limit_except_method : " << location->limit_except_method[k] << endl;
//             printConfig(link->link[i]->link[j]);
//             cout << "=============== location " << j << "===============" << endl;
//         }
//         cout << "=============== server " << i << " ===============" << endl;
//     }
//     cout << "=============== http ===============" << endl;
// }
#endif
