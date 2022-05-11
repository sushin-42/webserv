#include <cstdio>
#include <iostream>
#include <signal.h>
#include <poll.h>

#include "Config.hpp"
#include "ConnSocket.hpp"
#include "Poll.hpp"
#include "ResBody.hpp"
#include "ReqHeader.hpp"
#include "ResHeader.hpp"
#include "ServerSocket.hpp"



int main()
 {
	signal(SIGPIPE, SIG_IGN);

	ServerSocket		serv("", 8888);	// put your IP, "" means ANY
	ConnSocket			connected;
	ReqHeader			ReqH;
	ResHeader			ResH;
	ResBody				ResB;

	PollSet				set;
	PollSet::iterator	it;

	root += "/static_file";

	try						{ serv.bind(); }
	catch (exception& e)	{ cerr << e.what() << endl; }
	try						{ serv.listen(10 /*backlog*/); }
	catch (exception& e)	{ cerr << e.what() << endl; }

	set.enroll(&serv);
	while (1)
	{
		try						{ it = set.examine(serv); }
		catch (exception& e)	{ continue; }

		connected.setFD(it->fd);
		try						{ ReqH = connected.recvRequest(); }
		catch (exception& e)	{ connected.close(); set.drop(it); continue; }
		ReqH.setPath();
		ReqH.checkFile();
		ResH = makeResponseHeader(ReqH);

		cout << ReqH.path << " is Requested by " << it->fd << endl;
		switch (ReqH.getStatus())
		{
		case 200: ResB.readFile(ReqH.path);				break;
		case 404: ResB.readFile(root+"/404/404.html");	break;
		}
		connected.send(ResH.getContent());
		connected.send(ResB.getContent());
		connected.close();

		it->revents = 0;
		it->fd = -1;

		ReqH.clear();
		ResH.clear();
		ResB.clear();
	}
}






