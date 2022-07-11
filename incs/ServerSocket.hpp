#ifndef SERVERSOCKET_HPP
# define SERVERSOCKET_HPP

# include "ConnSocket.hpp"
# include <exception>
#include <map>
# include <stdexcept>
# include <sys/errno.h>
# include <sys/socket.h>


class ServerSocket : public ISocket
{
/**========================================================================
* %                           member variables
*========================================================================**/
public:
	vector<Config*> confs;
/**========================================================================
* @                           Constructors
*========================================================================**/

public:
	ServerSocket( const string&			ip,
				  const unsigned short& port )
	: ISocket(ip, port), confs() {};
	~ServerSocket() {};

/**========================================================================
* *                            operators
*========================================================================**/

/**========================================================================
* #                          member functions
*========================================================================**/

	void			bind();
	void			listen(int backlog);
	ConnSocket		accept() const;
	void 			send(const string& s);
	void 			core();
	void 			recv();
	void			coreDone();
	string			getOutputContent();
private:
	void dummy();
};


#endif
