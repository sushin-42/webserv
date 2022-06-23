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

	ServerSocket*		serv;
	ConnSocket*			connected;
	Pipe*				CGIpipe;
	FileStream*			filestream;
	Stream*			inputStream;
	Stream*			outputStream;

	PollSet				pollset;
	PollSet::iterator	it;

	string				content;
	map<int, undone>	writeUndoneBuf;
	createServerSockets(CONF->getAddrs(), pollset);
	pollset.createMonitor();

	while (1)
	{
//'--------------------------------CATCH EVENT-------------------------------'//
		try												{ whoDied();
														  it = pollset.examine(); }
		catch	(exception& e)							{ continue; }
		if		(CONVERT(*it.second, ServerSocket))		{ continue; }	// servSocket

		connected	= CONVERT(*it.second, ConnSocket);
		CGIpipe		= CONVERT(*it.second, Pipe);
		filestream	= CONVERT(*it.second, FileStream);

//.---------------------------------POLL OUT---------------------------------.//
		if (it.first->revents & POLLOUT)
		{
			it.first->events &= ~POLLOUT;
			if (CGIpipe)
			{
				outputStream = CGIpipe;
				connected = CGIpipe->linkConn;
				content = connected->ReqB.getContent();
				goto _send;
			}

			else if (filestream)
			{
				outputStream = filestream;
				connected = filestream->linkConn;
				content = connected->ReqB.getContent();
				goto _send;
			}

			else /* ConnSocket */
				goto _set_stream_to_socket;
		}
//#----------------------------------POLLIN----------------------------------#//
		else
		{
			if (CGIpipe)
			{
				connected =  CGIpipe->linkConn;
				inputStream = CGIpipe;
				goto _core;
			}

			else if (filestream)
			{
				connected =  filestream->linkConn;
				inputStream = filestream;
				goto _core;
			}

			else /* ConnSocket */
			{
				inputStream = connected;
			}

		}

//*-----------------------ConnSocket: parse request--------------------------*//
	try										{ connected->recvRequest(); }
	catch	(exception& e)					{
												if (CONVERT(&e, ConnSocket::connClosed) ||		//NOTE: what if client does not close after we send FIN? (cause we do graceful-close)
													CONVERT(&e, ConnSocket::somethingWrong))
												{
													connected->close();
													pollset.drop(it);
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
													goto _set_stream_to_socket;
												}
											}
//@---------------------------CORE: PROCESS INSTREAM-------------------------@//
_core:
		serv = connected->linkServerSock;
		try							{ core_wrapper(pollset, serv, inputStream); }	//@ make response header, body//
		catch (httpError& e)		{
									  redirectError* r = CONVERT(&e, redirectError);
									  if (r) {
												connected->ResH["Location"] = r->location;
											 }
										connected->returnError(e.status, e.what());
										it.first->events |= POLLOUT;
										continue;
								 	  goto _set_stream_to_socket;	}

		catch (exception& e)		{ if (CONVERT(&e, readMore)) continue; }


		if (inputStream)	/* reach here : READ DONE */
		{
			if (inputStream == connected)
			{
				if (connected->linkOutputFile)
					continue;
				// if (!connected->linkInputPipe)		/*  <------ pipe is just created, do not send 0 byte */
				pollset.getIterator(connected).first->events |= POLLOUT;
				continue;
			}
			if (inputStream == CGIpipe)
			{
				if (connected->pending == false)
					pollset.getIterator(connected).first->events |= POLLOUT;
				continue;
			}
			pollset.drop(it);

			connected->unlink(inputStream);
			pollset.getIterator(connected).first->events |= POLLOUT;
			continue;
		}


_set_stream_to_socket:
		outputStream	= connected;
		content = connected->ResH.getContent() +
				  connected->ResB.getContent();

//.-----------------------------SEND TO OUTSTREAM----------------------------.//
_send:
		try									{ outputStream->send(content, writeUndoneBuf); }
		catch (exception& e)				{
											  if		(CONVERT(&e, sendMore))	it.first->events |= POLLOUT;	// not all data sended
											  else if	(CONVERT(&e, readMore)) ;							 	// not all data sended, and have to read from pipe
											  else								{
											  									  if (outputStream == filestream)
																				  {
											  									  	pollset.getIterator(connected).first->events |= POLLOUT;
																				  	connected->unlink(outputStream);
																				  	pollset.drop(it);
																					continue;
																				  }
																				}

											}
		connected->ReqH.clear(), connected->ResH.clear(), connected->ResB.clear();
	}
}






