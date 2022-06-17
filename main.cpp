#include <algorithm>
#include <cstdio>
#include <iostream>
#include <signal.h>
#include <poll.h>
#include <string>
#include <sys/poll.h>


#include "CGI.hpp"
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

int main(int argc, char **argv)
{
	if (argvError(argc))
		return (errMsg());
	signal(SIGPIPE, SIG_IGN);
	// HttpConfig http;
	try
	{
		HttpConfig::getInstance()->setConfig(ReadConfig(argv));
		// pringConfigAll(HttpConfig::getInstance());
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
		return -1;
	}
	return 0;
	// IMPL: create all serverSocket
	ServerSocket serv("", 8888); // put your IP, "" means ANY
	ConnSocket *connected;
	Pipe *CGIpipe;
	IStream *stream;

	PollSet pollset;
	PollSet::iterator it;

	string content;
	map<int, undone> writeUndoneBuf;
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
		if (CONVERT(*it.second, ServerSocket))
		{
			continue;
		} // servSocket

		connected = CONVERT(*it.second, ConnSocket);
		CGIpipe = CONVERT(*it.second, Pipe);
		if (CGIpipe)
		{
			connected = CGIpipe->linkConn;
			if (CGIpipe == connected->linkWritePipe)
			{
				stream = CGIpipe;
				content = connected->ReqB.getContent();
				goto _send;
			}
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

			if (CONVERT(&e, IStream::readMore))
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
		if (!CGIpipe && connected->linkReadPipe) /* pipe is just created */
			continue;
		stream = connected;
		content = connected->ResH.getContent() +
				  connected->ResB.getContent();

	//.------------------------send response header, body------------------------.//
	_send:
		try
		{
			stream->send(content, writeUndoneBuf);
		}
		catch (exception &e)
		{
			if (CONVERT(&e, IStream::sendMore))
				it.first->events |= POLLOUT; // not all data sended
			else
				pollset.drop(it);
		}

		connected->ReqH.clear(), connected->ResH.clear(), connected->ResB.clear();
		//.---------------------------------send end---------------------------------.//
	}
}
