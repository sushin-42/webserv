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
	catch (exception& e)	{ cerr << e.what() << endl; exit(errno); }
	try						{ serv.listen(10 /*backlog*/); }
	catch (exception& e)	{ cerr << e.what() << endl; exit(errno); }

	set.enroll(&serv);
	while (1)
	{
//'----------------------catch and parse request header----------------------'//
		try							{ it = set.examine(serv); }
		catch	(exception& e)		{ continue; }
		if		(it == set.begin())	{ continue; }	// begin == servSocket

		connected.setFD(it->fd);
		if (it->revents & POLLOUT)	{ it->events &= ~POLLOUT;
									  goto resend; }

		try							{ ReqH = connected.recvRequest(); }
		catch	(exception& e)		{ continue; }
		if		(ReqH.empty())		{ connected.close();
									  set.drop(it);
									  continue; }	// client exit
//'-------------------------------- catch end--------------------------------'//

//@------------------------make response header, body------------------------@//
		ReqH.setHTTPversion("HTTP/1.1");
		ReqH.setRequsetTarget();
		ResH.setHTTPversion("HTTP/1.1");
		ResH.setStatusCode(checkFile(root + ReqH.getRequsetTarget()));

		switch (ResH.getStatusCode())
		{
		case 200:
			ResH.setReasonPhrase("OK");
			ResB.readFile(root + ReqH.getRequsetTarget());
			break;
		case 404:
			ResH.setReasonPhrase("Not Found");
			ResB.readFile(root + "/404/404.html");
			break;
		}
		ResH.makeStatusLine();

		ResH["Content-Type"]	= MIME[getExt(ReqH.getRequsetTarget())];
		ResH["Connection"]		= "close";
		ResH["Content-Length"]	= toString(ResB.getContent().length());
		ResH.integrate();
//@---------------------------------make end---------------------------------@//

//.------------------------send response header, body------------------------.//
resend:
		try						{ connected.send(ResH.getContent() + ResB.getContent(), undoneBuf); }
		catch (exception& e)	{ it->events |= POLLOUT; }	// not all data sended

		ReqH.clear(), ResH.clear(), ResB.clear();
//.---------------------------------send end---------------------------------.//
	}
}






