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
#include "core.hpp"



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
		try								{ it = set.examine(); }
		catch	(exception& e)			{ continue; }
		if		((*it.second) == &serv)	{ continue; }	// servSocket

		connected = dynamic_cast<ConnSocket*>(*it.second);

		if (it.first->revents & POLLOUT)	{ it.first->events &= ~POLLOUT;
									  		  goto resend; }

		try							{ Req = connected->recvRequest(); ReqH = Req.first; ReqB = Req.second; }
		catch	(exception& e)		{ continue; }
		if		(ReqH.empty())		{ connected->close();
									  set.drop(it);
									  continue; }	// client exit

//'-------------------------------- catch end--------------------------------'//

		core_wrapper(&serv, connected, ReqH, ReqB, ResH, ResB);	//@ make response header, body//

//.------------------------send response header, body------------------------.//
resend:
		try						{ connected->send(ResH.getContent() + ResB.getContent(), undoneBuf); }
		catch (exception& e)	{ it.first->events |= POLLOUT; }	// not all data sended

		ReqH.clear(), ResH.clear(), ResB.clear();
//.---------------------------------send end---------------------------------.//
	}
}






