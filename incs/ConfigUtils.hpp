#ifndef CONFIGUTILS_HPP
#define CONFIGUTILS_HPP

#include "ConfigUtilsException.hpp"
#include <sstream>
#include <fstream>
#include <arpa/inet.h>

using namespace std;

typedef pair<string, unsigned short> _Addr;

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

bool exsitErrorpageEqual(string arg, int &equalstatus);
//=====================================parse_util========================================
//=====================================parse_arg========================================

void parse_root(vector<string> arg, Config *config);
void parse_listen(vector<string> arg, Config *config);
void parse_server_names(vector<string> arg, Config *config);
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
void parse_cgi(vector<string> arg, Config *config);
void parse_file_access(vector<string> arg, Config *config);
void parse_timer(vector<string> arg, Config *config);
void parse_return(vector<string> arg, Config *config);
void parse_server_name_in_redirect(vector<string> arg, Config *config);
void parse_port_in_redirect(vector<string> arg, Config *config);
//=====================================parse_arg========================================

void EraseComment(string &configtemp);
string ReadConfig(int argc, char **argv);


#endif
