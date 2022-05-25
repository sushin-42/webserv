#include <cstdio>
#include <iostream>
#include <signal.h>
#include <poll.h>

#include "CGI.hpp"
#include "Config.hpp"
#include "ConnSocket.hpp"
#include "Poll.hpp"
#include "ReqBody.hpp"
#include "ResBody.hpp"
#include "ReqHeader.hpp"
#include "ResHeader.hpp"
#include "ServerSocket.hpp"



int main()
 {
	signal(SIGPIPE, SIG_IGN);

	ServerSocket				serv("", 8888);	// put your IP, "" means ANY
	ConnSocket*					connected;
	pair<ReqHeader, ReqBody>	Req;
	ReqHeader					ReqH;
	ReqBody						ReqB;
	ResHeader					ResH;
	ResBody						ResB;

	PollSet				set;
	PollSet::iterator	it;

	map<string, string>::const_iterator mit;
	map<string, string>::const_iterator mite;

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

		connected = dynamic_cast<ConnSocket*>(*it.second);

		if (it.first->revents & POLLOUT)	{ it.first->events &= ~POLLOUT;
									  		  goto resend; }

		try							{ Req = connected->recvRequest(); ReqH = Req.first; ReqB = Req.second; }
		catch	(exception& e)		{ continue; }
		if		(ReqH.empty())		{ connected->close();
									  set.drop(it);
									  continue; }	// client exit

//'-------------------------------- catch end--------------------------------'//

//@------------------------make response header, body------------------------@//
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
		if (getExt(ReqH.getRequsetTarget()) == "py")
			CGIRoutines(ReqH, ReqB, ResH, ResB);

		ResH["Content-Type"]	= MIME[getExt(ReqH.getRequsetTarget())];
		ResH["Connection"]		= "close";
		ResH["Content-Length"]	= toString(ResB.getContent().length());
		ResH.integrate();
//@---------------------------------make end---------------------------------@//


//.------------------------send response header, body------------------------.//
resend:
		try						{ connected->send(ResH.getContent() + ResB.getContent(), undoneBuf); }
		catch (exception& e)	{ it.first->events |= POLLOUT; }	// not all data sended

		ReqH.clear(), ResH.clear(), ResB.clear();
//.---------------------------------send end---------------------------------.//
	}
}






