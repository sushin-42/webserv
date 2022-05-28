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

	ServerSocket		serv("", 8888);	// put your IP, "" means ANY
	ConnSocket*			connected;

	PollSet				pollset;
	PollSet::iterator	it;

	map<int, undone>	undoneBuf;
	root += "/static_file";

	try						{ serv.bind(); }
	catch (exception& e)	{ cerr << e.what() << endl; exit(errno); }
	try						{ serv.listen(10 /*backlog*/); }
	catch (exception& e)	{ cerr << e.what() << endl; exit(errno); }

	pollset.enroll(&serv);
	while (1)
	{
//'----------------------catch and parse request header----------------------'//
		try									{ it = pollset.examine(); }
		catch	(exception& e)				{ continue; }
		if		((*it.second) == &serv)		{ continue; }	// servSocket

		connected = dynamic_cast<ConnSocket*>(*it.second);

		if (it.first->revents & POLLOUT)	{ it.first->events &= ~POLLOUT;
									  		  goto resend; }

		try									{ connected->recvRequest();}
		catch	(exception& e)				{ continue; }
		if		(connected->ReqH.empty())	{ connected->close();
									  		  pollset.drop(it);
									  		  continue; }	// client exit

//'-------------------------------- catch end--------------------------------'//

		core_wrapper(&serv, connected);	//@ make response header, body//

//.------------------------send response header, body------------------------.//
resend:
		try									{ connected->send(
																connected->ResH.getContent() +
																connected->ResB.getContent(),
																undoneBuf
															);
											}
		catch (exception& e)				{ it.first->events |= POLLOUT; }	// not all data sended

		connected->ReqH.clear(), connected->ResH.clear(), connected->ResB.clear();
//.---------------------------------send end---------------------------------.//
	}
}






