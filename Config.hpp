#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <string>
# include <unistd.h>
# include <vector>
# include "utils.hpp"
using namespace std;


string root = getcwd(NULL, 0);

map<string, string>	MIME= getMIME();

//HttpConfig http;

/* in main context */
// map<string, string>	env;
// string				user;
// string				working_directory;


// COMMON_CONFIG_TOKEN = {
// "root",
// "auto_index",
// "error_page",
// "keepalive_requests",
// "default_type",
// "client_body_size",
// "reset_timedout_connection",
// "lingering_timeout",
// "lingering_time",
// "keepalive_time",
// "keepalive_timeout",
// "send_timeout",
// "client_body_timeout"
// };


class ErrorPage{};
class Config
{
/**========================================================================
* %                          member variables
*========================================================================**/

protected:
	vector<Config*>		link;

	string				root;
	bool				auto_index;		//directory listing
	ErrorPage			error_page;
	int					keepalive_requests;

	string				default_type;
	int					client_body_size;
	bool				reset_timedout_connection;

	time_t				lingering_timeout;
	time_t				lingering_time;
	time_t				keepalive_time;
	time_t				keepalive_timeout;
	time_t				send_timeout;
	time_t				client_body_timeout;

	// bool				absolute_redirect;
	// bool				server_name_in_redirect;
	// bool				port_in_redirect;

/**========================================================================
* @                           Constructors

*========================================================================**/
public:
	Config() : link() {}
	Config( const Config& src ) : link(src.link) {}
	virtual ~Config() {}

/**========================================================================
* *                            operators
*========================================================================**/

	Config&	operator=( const Config& src )
	{
		if (this != &src)
		{
			this->link = src.link;
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



//*--------------------------------------------------------------------------*//





class HttpConfig : public Config
{
/**========================================================================
* @                           Constructors
*========================================================================**/

public:
	HttpConfig() : Config() {}
	HttpConfig( const HttpConfig& src ) : Config() {}
	virtual ~HttpConfig() {}

/**========================================================================
* *                            operators
*========================================================================**/

	HttpConfig&	operator=( const Config& src )
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





//*--------------------------------------------------------------------------*//





class ServerConfig : public Config
{
/**========================================================================
* %                          member variables
*========================================================================**/

private:
	string			server_name;
	string			ip;
	unsigned short	port;

/**========================================================================
* @                           Constructors
*========================================================================**/

public:
	ServerConfig() : Config() {}
	ServerConfig( const ServerConfig& src ) : Config() {}
	virtual ~ServerConfig() {}

/**========================================================================
* *                            operators
*========================================================================**/

	ServerConfig&	operator=( const ServerConfig& src )
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




//*--------------------------------------------------------------------------*//





class LocationConfig : public Config
{
/**========================================================================
* %                          member variables
*========================================================================**/

	string				URI;
	bool				assign;
	vector<string>		limit_except_method;

/**========================================================================
* @                           Constructors
*========================================================================**/

public:
	LocationConfig() : Config() {}
	LocationConfig( const LocationConfig& src ) : Config() {}
	virtual ~LocationConfig() {}

/**========================================================================
* *                            operators
*========================================================================**/

	LocationConfig&	operator=( const Config& src )
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
