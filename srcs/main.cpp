#include <algorithm>
#include <cstdio>
#include <iostream>
#include <signal.h>
#include <poll.h>
#include <string>
#include <sys/poll.h>

#include "CGI.hpp"
#include "Config.hpp"
#include "ConfigLoader.hpp"
#include "FileStream.hpp"
#include "HttpConfig.hpp"
#include "ConfigUtils.hpp"
#include "ConnSocket.hpp"
#include "Pipe.hpp"
#include "Poll.hpp"
#include "ReqBody.hpp"
#include "ResBody.hpp"
#include "ReqHeader.hpp"
#include "ResHeader.hpp"
#include "ServerSocket.hpp"
#include "core.hpp"
#include "httpError.hpp"
#include "utils.hpp"
#include "Exceptions.hpp"


map<string, string> MIME;
// void	setOutputStream(Stream* value, Stream** outputStream)
// {
// 	ConnSocket*	connected;
// 	*outputStream = value;
// 	if (CONVERT(value, Pipe))
// 	// connected = Stream->linkConn;
// 	// content = connected->ReqB.getContent();
// }
// void	setInputStream()
// {}

int main(int argc, char** argv)
 {
	if (argvError(argc))
		return (errMsg());
	signal(SIGPIPE, SIG_IGN);
	// HttpConfig http;
	try
	{
		HttpConfig::getInstance()->setConfig(ReadConfig(argv));
		HttpConfig::getInstance()->defaultSet();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
		return -1;
	}

	CONF->setAddrs(HttpConfig::getInstance()->serverMap);
	CONF->loadMIME();

	ConnSocket*			connected;
	Pipe*				CGIpipe;
	FileStream*			filestream;
	Stream*				inputStream;
	Stream*				outputStream;

	PollSet::iterator	it;

	string				content;
	map<int, undone>	writeUndoneBuf;
	createServerSockets(CONF->getAddrs());
	POLLSET->createMonitor();

	while (1)
	{
//'--------------------------------CATCH EVENT-------------------------------'//
		try												{ whoDied();
														  it = POLLSET->examine(); }
		catch	(exception& e)							{ continue; }
		if		(CONVERT(*it.second, ServerSocket))		{ continue; }	// servSocket

		connected	= CONVERT(*it.second, ConnSocket);
		CGIpipe		= CONVERT(*it.second, Pipe);
		filestream	= CONVERT(*it.second, FileStream);

		if (CGIpipe)	connected=CGIpipe->linkConn;
		if (filestream)	connected=filestream->linkConn;

//.---------------------------------POLL OUT---------------------------------.//
		if (it.first->revents & POLLOUT)
		{
			it.first->events &= ~POLLOUT;
			outputStream = *it.second;
			if (outputStream == connected)
				content = connected->ResH.getContent() +
						  connected->ResB.getContent();
			else
				content = connected->ReqB.getContent();
			goto _send;
		}
//#----------------------------------POLLIN----------------------------------#//
		else
		{
			inputStream = *it.second;
			if (inputStream != connected)
				goto _core;
		}

//*-----------------------ConnSocket: parse request--------------------------*//
	try										{ connected->recv(); }
	catch	(exception& e)					{
												if (CONVERT(&e, ConnSocket::connClosed) ||		//NOTE: what if client does not close after we send FIN? (cause we do graceful-close)
													CONVERT(&e, ConnSocket::somethingWrong))
												{
													connected->close();
													POLLSET->drop(it);
													continue;
												}

												if (CONVERT(&e, readMore))
												{
													continue;
												}

												httpError* err = CONVERT(&e, httpError);
												if (err)
												{
													connected->returnError(err->status, err->what());
													it.first->events |= POLLOUT;
													continue;
												}
											}
//@---------------------------CORE: PROCESS INSTREAM-------------------------@//
_core:
		try							{ core_wrapper(inputStream); }	//@ make response header, body//
		catch (httpError& e)		{
									  redirectError* r = CONVERT(&e, redirectError);
									  if (r) { connected->ResH["Location"] = r->location; }
									  connected->returnError(e.status, e.what());
									  it.first->events |= POLLOUT;
									  continue;
									}

		catch (exception& e)		{ if (CONVERT(&e, readMore)) continue; }


		if (inputStream)		/* reach here : READ DONE */
		{
			if (inputStream == connected)
			{
				if (connected->linkOutputFile)
					continue;
				// if (!connected->linkInputPipe)		/*  <------ pipe is just created, do not send 0 byte */
				POLLSET->getIterator(connected).first->events |= POLLOUT;
				continue;
			}
			else if (inputStream == CGIpipe)
			{
				if (connected->pending == false)
					POLLSET->getIterator(connected).first->events |= POLLOUT;
				continue;
			}
			else
			{
				POLLSET->drop(it);
				connected->unlink(inputStream);
				POLLSET->getIterator(connected).first->events |= POLLOUT;
				continue;
			}
		}

//.-----------------------------SEND TO OUTSTREAM----------------------------.//
_send:
		try									{ outputStream->send(content, writeUndoneBuf); }
		catch (exception& e)				{
											  if		(CONVERT(&e, sendMore))	it.first->events |= POLLOUT;	// not all data sended
											  else if	(CONVERT(&e, readMore)) ;							 	// not all data sended, and have to read from pipe
											  else		{
															if (outputStream != connected)
															{
																if (outputStream == filestream)
																	POLLSET->getIterator(connected).first->events |= POLLOUT;
																connected->unlink(outputStream);
																POLLSET->drop(it);
																continue;
															}
															// POLLSET->drop(it);
														}

											}
		connected->ReqH.clear(), connected->ResH.clear(), connected->ResB.clear();
	}
}
