#include <algorithm>
#include <cstdio>
#include <iostream>
#include <ostream>
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
#include "HTTP_Error.hpp"
#include "utils.hpp"
#include "Exceptions.hpp"
#include "WriteUndoneBuf.hpp"


//**------------------------------------------------------------------------
//                                  TODO                                   .
//*
//*		1. CGI stderr
//*
//*------------------------------------------------------------------------**/

map<string, string> MIME;

int main(int argc, char** argv)
 {
	if (argvError(argc))
		return (errMsg());
	signal(SIGPIPE, SIG_IGN);

	try							{ HttpConfig::getInstance()->setConfig(ReadConfig(argc, argv));}
	catch (const exception &e)	{ cerr << e.what() << endl; return -1; }

	Pipe*						CGIpipe;
	ConnSocket*					connected;
	FileStream*					filestream;

	Stream*						inputStream;
	Stream*						outputStream;

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
			string	outputContent;
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

			if (POLLSET->getCatchedEvent(stream->getFD()) & POLLOUT)
			{
				POLLSET->unsetSend(stream->getFD());
				if (stream == connected && connected->internalRedirect)
				{
					inputStream = connected;
					goto _core;
				}
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
			catch	(HTTP_Error& h)		{
											connected->returnError(h);
											POLLSET->prepareSend( connected->getFD() );
											continue;
										}
			catch	(exception& e)		{
											if (CONVERT(&e, ConnSocket::connClosed) ||
												CONVERT(&e, ConnSocket::somethingWrong))
											{
												connected->unlinkAll();
												POLLSET->drop(connected);

												continue;
											}
										}

//@----------------------------------RECV DONE-------------------------------@//

//%--------------------------CORE: process before send-----------------------%//
_core:
			try								{	inputStream->core();	}
			catch	(readMore& r)			{	continue;	 }
			catch	(internalRedirect& r)	{	connected->internalRedirect=true;
												POLLSET->prepareSend( connected->getFD() );	/* make ConnSocket to be catched in PollSet#examine() */
												continue;
											}
			catch	(HTTP_Error& h)			{
												connected->returnError(h);
												POLLSET->prepareSend( connected->getFD() );
												continue;
											}
			catch	(autoIndex& a)			{	POLLSET->prepareSend( connected->getFD() ); /* go down ? */	 }

			inputStream->coreDone();
			continue;

//%--------------------------------READY TO SEND-----------------------------%//

//.-----------------------------SEND TO OUTSTREAM----------------------------.//
_send:
		if (outputStream == connected)
		{
			try								{ connected->checkErrorPage(); }
			catch (internalServerError& e) 	{ connected->returnError(e); outputContent = connected->getOutputContent();}
			catch (internalRedirect& e) 	{ connected->internalRedirect=true;
											  POLLSET->prepareSend( connected->getFD() );	/* make ConnSocket to be catched in PollSet#examine() */
											  continue;
											}
		}

			try							{ outputStream->send(outputContent);}
			catch (exception& e)		{
											if		(CONVERT(&e, sendMore))	{POLLSET->prepareSend( outputStream->getFD() ); continue;}	// not all data sended
											else if	(CONVERT(&e, readMore)) {continue;}	// not all data sended, and have to read from pipe
											else if	(CONVERT(&e, gotoCore)) { inputStream=connected; goto _core;}	// Connection was keep-alive, data received remained.
											else							{}
										}
		}
	}
}
