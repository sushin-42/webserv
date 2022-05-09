#include <iostream>
#include <signal.h>

#include "Config.hpp"
#include "ResBody.hpp"
#include "ReqHeader.hpp"
#include "ResHeader.hpp"
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
		ReqHeader	reqH;
		ResHeader	resH;
		ResBody		resB;
		connected	= serv.accept();
		reqH		= connected.recvRequest();
		reqH.setPath();
		reqH.checkFile();

		resH = makeResponseHeader(reqH);
		connected.send(resH.getContent());
		if (reqH.status == 200)
		{
			resB.readFile(reqH.path);
			connected.send(resB.getContent());
		}
		// reqH.clear();
		// resH.clear();
		// reqB.clear();
	}
}


