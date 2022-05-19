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

	map<int, undone>	undoneBuf;
	root += "/static_file";

	try						{ serv.bind(); }
	catch (exception& e)	{ cerr << e.what() << endl; }
	try						{ serv.listen(10 /*backlog*/); }
	catch (exception& e)	{ cerr << e.what() << endl; }

	set.enroll(&serv);
	while (1)
	{
		try							{ it = set.examine(serv); }
		catch	(exception& e)		{ continue; }
		if		(it == set.begin())	{ continue; }	// begin == servSocket

		connected.setFD(it->fd);
		if (it->revents & POLLOUT)	{ it->events &= ~POLLOUT;
									  goto resend; }

//@							make response header, body						@//
		try							{ ReqH = connected.recvRequest(); }
		catch	(exception& e)		{ continue; }
		if		(ReqH.empty())		{ connected.close();
									  set.drop(it);
									  continue; }	// client exit

		ReqH.setPath(),	ReqH.checkFile();

		switch (ReqH.getStatus())
		{
		case 200: ResB.readFile(ReqH.path);				break;
		case 404: ResB.readFile(root+"/404/404.html");	break;
		}
		ResH["http-version"]	= "HTTP/1.1";
		ResH["status-code"]		= ReqH.getStatus();
		ResH["reason-phrase"]	= ReqH.reason;
		ResH.makeStatusLine();

		ResH["Content-Type"]	= MIME[getExt(ReqH.path)];
		ResH["Connection"]		= "close";
		ResH["Content-Length"]	= toString(ResB.getContent().length());
		ResH.integrate();
//@									make end								@//

resend:
		try						{ connected.send(ResH.getContent(), undoneBuf); }
		catch (exception& e)	{ it->events |= POLLOUT; }	// not all data sended
		try						{ connected.send(ResB.getContent(), undoneBuf); }
		catch (exception& e)	{ it->events |= POLLOUT; }	// not all data sended

		ReqH.clear(), ResH.clear(), ResB.clear();
	}
}






