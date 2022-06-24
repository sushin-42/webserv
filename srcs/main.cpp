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
//**------------------------------------------------------------------------
//                                  TODO                                   .
//*
//*		1. drop link
//*		2. error_page
//*		3. CGI stderr
//*		4. CGI local-redir
//*
//*------------------------------------------------------------------------**/

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

	Pipe*						CGIpipe;
	ConnSocket*					connected;
	FileStream*					filestream;

	Stream*						inputStream;
	Stream*						outputStream;
	string						outputContent;

	map<int, undone>			writeUndoneBuf;
	vector<Stream*>				ret;
	vector<Stream*>::iterator	it, ite;
	Stream*						stream;

	CONF->setAddrs(HttpConfig::getInstance()->serverMap);
	CONF->loadMIME();
	POLLSET->createMonitor();
	createServerSockets(CONF->getAddrs());

	while (1)
	{
//'--------------------------------CATCH EVENT-------------------------------'//
		try								{ whoDied();
										  ret = POLLSET->examine(); }
		catch	(exception& e)			{ continue; }


		it = ret.begin(), ite = ret.end();
		for ( ; it < ite; it++)
		{
			stream = *it;
			if	(CONVERT(stream, ServerSocket))	{ continue; }

			connected	= CONVERT(stream, ConnSocket);
			CGIpipe		= CONVERT(stream, Pipe);
			filestream	= CONVERT(stream, FileStream);

			if (CGIpipe)				{ connected = CGIpipe->linkConn; }
			if (filestream)				{ connected = filestream->linkConn;}
			if (connected == NULL)		{ POLLSET->drop(stream); continue; }		//NOTE: unlinked by drop(ConnSocket)
																					//NOTE: what if Pipe | FileStream not in ret?
//.---------------------------------POLL OUT---------------------------------.//

			if (POLLSET->pollMap[stream->getFD()].first.revents & POLLOUT)
			{
				POLLSET->pollMap[stream->getFD()].first.events &= ~POLLOUT;
				outputStream = stream;
				outputContent = outputStream->getOutputContent();
				goto _send;
			}

//#----------------------------------POLLIN----------------------------------#//

			else
				inputStream = stream;

//@---------------------------RECV FROM INPUT STREAM-------------------------@//

			try							{	inputStream->recv();	}
			catch	(readMore& r)		{	continue; }
			catch	(httpError& h)		{
											connected->returnError(h);
											POLLSET->pollMap[stream->getFD()].first.events |= POLLOUT;	//NOTE: POLLOUT CONNSOCK ?
											continue;
										}
			catch	(exception& e)		{
											if (CONVERT(&e, ConnSocket::connClosed) ||
												CONVERT(&e, ConnSocket::somethingWrong))
											{
												connected->close();
												POLLSET->drop(stream);

												continue;
											}
										}

//@----------------------------------RECV DONE-------------------------------@//

//%--------------------------CORE: process before send-----------------------%//
			try							{	inputStream->core();	}
			catch	(readMore& r)		{	continue;	 }
			catch	(httpError& h)		{
											connected->returnError(h);
											POLLSET->pollMap[stream->getFD()].first.events |= POLLOUT;	//NOTE: POLLOUT CONNSOCK ?
											continue;
										}
			inputStream->coreDone();
			connected->makeResponseHeader();
			if (inputStream == filestream)
				POLLSET->drop(stream);
			continue;

//%--------------------------------READY TO SEND-----------------------------%//

//.-----------------------------SEND TO OUTSTREAM----------------------------.//
// _send:
// 		try							{ outputStream->send(outputContent, writeUndoneBuf); }
// 		catch (exception& e)		{
// 										if		(CONVERT(&e, sendMore))	POLLSET->pollMap[stream->getFD()].first.events |= POLLOUT;	// not all data sended
// 										else if	(CONVERT(&e, readMore)) ;							 	// not all data sended, and have to read from pipe
// 										else	{
// 													if (outputStream != connected)
// 													{
// 														if (outputStream == filestream)
// 															POLLSET->getIterator(connected).first->events |= POLLOUT;
// 														connected->unlink(outputStream);
// 														POLLSET->drop(stream);
// 														continue;
// 													}
// 													// POLLSET->drop(stream);
// 												}
// 									}
// 		connected->ReqH.clear(), connected->ResH.clear(), connected->ResB.clear();

_send:
			try							{ outputStream->send(outputContent, writeUndoneBuf);connected->ReqH.clear(), connected->ResH.clear(), connected->ResB.clear();}
			catch (exception& e)		{
											if		(CONVERT(&e, sendMore))	{POLLSET->pollMap[stream->getFD()].first.events |= POLLOUT; connected->ReqH.clear(), connected->ResH.clear(), connected->ResB.clear(); continue;}	// not all data sended
											else if	(CONVERT(&e, readMore)) {connected->ReqH.clear(), connected->ResH.clear(), connected->ResB.clear(); continue;}							 	// not all data sended, and have to read from pipe
											else	{
														if (outputStream != connected)
														{
															if (outputStream == filestream)
																POLLSET->pollMap[connected->getFD()].first.events |= POLLOUT;
															connected->unlink(outputStream);
															POLLSET->drop(stream);
															continue;
														}
														connected->ReqH.clear(), connected->ResH.clear(), connected->ResB.clear();
														continue;
													}
										}
			POLLSET->drop(stream);
		}
	}
}
