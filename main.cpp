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

#define SIZE 10

int main()
 {
	signal(SIGPIPE, SIG_IGN);

	ServerSocket		serv("", 8888);	// put your IP, "" means ANY
	ConnSocket			connected;
	ReqHeader			ReqH;
	ResHeader			ResH;
	ResBody				ResB;

	PollSet				set;
	// ISocket*			ret = NULL;
	Poll*			ret = NULL;

	root += "/static_file";

	try						{ serv.bind(); }
	catch (exception& e)	{ cerr << e.what() << endl; }
	try						{ serv.listen(0); }
	catch (exception& e)	{ cerr << e.what() << endl; }

	Poll sp = Poll(&serv);
	set.enroll(sp);
	while (1)
	{
		ret = set.examine(serv);
		if (ret)
		{
			cout << "show ret! " <<  ret->s << " : " <<  ret->s->getFD() << endl;
			ReqH = ((ConnSocket*)(ret->s))->recvRequest();
			ReqH.setPath();
			ReqH.checkFile();
			ResH = makeResponseHeader(ReqH);

			cout << ReqH.path << " is Requested by " << ret->s->getFD() << endl;

			switch (ReqH.status)
			{
			case 200: ResB.readFile(ReqH.path);				break;
			case 404: ResB.readFile(root+"/404/404.html");	break;
			}

			((ConnSocket*)(ret->s))->send(ResH.getContent());
			((ConnSocket*)(ret->s))->send(ResB.getContent());
			// ret->s->close();
			ret->revents = 0;
			ret->fd = -1;
		}
	}
 }
