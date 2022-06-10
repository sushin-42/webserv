#include <algorithm>
#include <cstdio>
#include <iostream>
#include <signal.h>
#include <poll.h>
#include <sys/poll.h>

#include "CGI.hpp"
// #include "Config.hpp"
#include "ConnSocket.hpp"
#include "Pipe.hpp"
#include "Poll.hpp"
#include "ReqBody.hpp"
#include "ResBody.hpp"
#include "ReqHeader.hpp"
#include "ResHeader.hpp"
#include "ServerSocket.hpp"
#include "core.hpp"
#include "utils.hpp"
#include "LocationConfig.hpp"
#include "ServerConfig.hpp"
#include "HttpConfig.hpp"
int main(int argc, char **argv)
{

	signal(SIGPIPE, SIG_IGN);
	argc = 0;
	HttpConfig http(argv);
	ServerSocket serv("", 8888); // put your IP, "" means ANY
	ConnSocket *connected;
	Pipe *CGIpipe;

	PollSet pollset;
	PollSet::iterator it;

	map<int, undone> undoneBuf;
	root += "/static_file";

	try
	{
		serv.bind();
	}
	catch (exception &e)
	{
		cerr << e.what() << endl;
		exit(errno);
	}
	try
	{
		serv.listen(10 /*backlog*/);
	}
	catch (exception &e)
	{
		cerr << e.what() << endl;
		exit(errno);
	}

	pollset.createMonitor();
	pollset.enroll(&serv);
	while (1)
	{
		//'----------------------catch and parse request header----------------------'//
		try
		{
			whoDied();
			it = pollset.examine();
		}
		catch (exception &e)
		{
			continue;
		}
		if ((*it.second) == &serv)
		{
			continue;
		} // servSocket

		connected = CONVERT(*it.second, ConnSocket);
		CGIpipe = CONVERT(*it.second, Pipe);
		if (CGIpipe)
		{
			connected = CGIpipe->linkConn;
			goto _core;
		}

		if (it.first->revents & POLLOUT)
		{
			it.first->events &= ~POLLOUT;
			goto _send;
		}

		try
		{
			connected->recvRequest();
		}
		catch (exception &e)
		{

			if (CONVERT(&e, ConnSocket::connClosed) || // NOTE: what if client does not close after we send FIN? (cause we do graceful-close)
				CONVERT(&e, ConnSocket::somethingWrong))
			{
				connected->close();
				pollset.drop(it);
				continue;
			}

			if (CONVERT(&e, ConnSocket::readMore))
			{
				continue;
			}

			if (CONVERT(&e, ConnSocket::badRequest)) // TODO: we need to close connection. not just SHUT_WR!
			{
				/* clear all existing? */
				connected->setErrorPage(400, "Bad Request", "Bad Request");
				connected->ResH.setDefaultHeaders();
				connected->ResH.makeStatusLine();
				connected->ResH.integrate();
				goto _send;
			}
			if (CONVERT(&e, ConnSocket::methodNotAllowed))
			{
				/* clear all existing? */
				connected->setErrorPage(405, "Method Not Allowed", "Method Not Allowed");
				connected->ResH.setDefaultHeaders();
				connected->ResH.makeStatusLine();
				connected->ResH.integrate();
				goto _send;
			}
		}

	//'-------------------------------- catch end--------------------------------'//
	_core:
		core_wrapper(pollset, &serv, connected, CGIpipe); //@ make response header, body//
		if (connected->pending)
			continue;

	//.------------------------send response header, body------------------------.//
	_send:
		try
		{
			connected->send(
				connected->ResH.getContent() +
					connected->ResB.getContent(),
				undoneBuf);
		}
		catch (exception &e)
		{
			it.first->events |= POLLOUT;
		} // not all data sended

		connected->ReqH.clear(), connected->ResH.clear(), connected->ResB.clear();
		//.---------------------------------send end---------------------------------.//
	}
}
