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

int main(int argc, char** argv)
 {
	if (argvError(argc))
		return (errMsg());
	signal(SIGPIPE, SIG_IGN);
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


	ConnSocket*			connected;
	Pipe*				CGIpipe;
	FileStream*			filestream;
	Stream*				inputStream;
	Stream*				outputStream;

	PollSet::iterator	it;

	string				outputContent;
	map<int, undone>	writeUndoneBuf;

	CONF->setAddrs(HttpConfig::getInstance()->serverMap);
	CONF->loadMIME();
	POLLSET->createMonitor();
	createServerSockets(CONF->getAddrs());

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
			outputContent = outputStream->getOutputContent();
			goto _send;
		}
//#----------------------------------POLLIN----------------------------------#//
		else
			inputStream = *it.second;

//@---------------------------RECV FROM INPUT STREAM-------------------------@//

	try										{ inputStream->recv(); }
	catch	(readMore& r)					{ continue; }
	catch	(httpError& h)					{
												connected->returnError(h);
												it.first->events |= POLLOUT;
												continue;
											}
	catch	(exception& e)					{
												if (CONVERT(&e, ConnSocket::connClosed) ||		//NOTE: what if client does not close after we send FIN? (cause we do graceful-close)
													CONVERT(&e, ConnSocket::somethingWrong))
												{
													connected->close();
													POLLSET->drop(it);
													continue;
												}
											}
//@----------------------------------RECV DONE-------------------------------@//

//%--------------------------CORE: process before send-----------------------%//
		try							{	inputStream->core();	}
		catch	(readMore& r)		{	continue;	 }
		catch	(httpError& h)		{
										connected->returnError(h);
										it.first->events |= POLLOUT;
										continue;
									}
		inputStream->coreDone();
		connected->makeResponseHeader();
		if (inputStream == filestream)
			POLLSET->drop(it);
		continue;

//%--------------------------------READY TO SEND-----------------------------%//

//.-----------------------------SEND TO OUTSTREAM----------------------------.//
_send:
		try									{ outputStream->send(outputContent, writeUndoneBuf); }
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
