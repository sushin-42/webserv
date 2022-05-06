#include <iostream>
#include <signal.h>

#include "Config.hpp"
#include "HeaderTemplate.hpp"
#include "ServerSocket.hpp"

int main()
 {
	signal(SIGPIPE, SIG_IGN);

	ServerSocket		serv("", 8888);	// put your IP, "" means ANY
	ConnSocket			connected;

	root += "/static_file";

	try						{ serv.bind(); }
	catch (exception& e)	{ cerr << e.what() << endl; }
	try						{ serv.listen(0); }
	catch (exception& e)	{ cerr << e.what() << endl; }

	while (1)
	{
		connected = serv.accept();
		cout << connected.getRequest() << endl;
		connected.sendResponse();
	}
}


