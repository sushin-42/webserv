# include "ConnSocket.hpp"
# include "CGI.hpp"
# include "Exceptions.hpp"
#include "ResBody.hpp"
#include "HTTP_Error.hpp"
#include "WriteUndoneBuf.hpp"
# include "utils.hpp"
# include "ServerConfig.hpp"
# include "ConfigLoader.hpp"
# include "ConfigChecker.hpp"
# include "checkFile.hpp"
# include <exception>
# include <string>
# include "Poll.hpp"




/**========================================================================
* @                           Constructors
*========================================================================**/

	ConnSocket::ConnSocket()
	: ISocket(), len(sizeof(info)),
	  recvContent(), ReqH(), ReqB(), ResH(), ResB(),
	  pending(true), chunk(false), FINsended(false), internalRedirect(false), keepAlive(true),
	  internalRedirectCount(0), currentReqCount(0),
	  linkInputPipe(NULL), linkOutputPipe(NULL),
	  linkInputFile(NULL), linkOutputFile(NULL),
	  linkServerSock(NULL), conf(NULL),
	  serverName() {}

	ConnSocket::~ConnSocket() {}

/**========================================================================
* *                            operators
*========================================================================**/

	ConnSocket&	ConnSocket::operator=( const ConnSocket& src )
	{
		if (this != &src)
		{
			this->ISocket::operator=(src);
			this->ReqH		= src.ReqH;
			this->ReqB		= src.ReqB;
			this->ResH		= src.ResH;
			this->ResB		= src.ResB;
			this->pending	= src.pending;
			this->chunk		= src.chunk;
			this->FINsended	= src.FINsended;
			this->linkInputPipe	= src.linkInputPipe;
			this->linkOutputPipe	= src.linkOutputPipe;
			this->linkInputFile	= src.linkInputFile;
			this->linkOutputFile	= src.linkOutputFile;
			this->linkServerSock= src.linkServerSock;
			this->conf			= src.conf;
		}
		return *this;
	}

/**========================================================================
* #                          member functions
*========================================================================**/

	void	ConnSocket::core()
	{
		struct stat		s;
		string			uriPath;

		string			filename;
		string			ext;
		string			CGIexecutable;
		string			method;

		if (this->internalRedirect == true)
		{
			internalRedirect = false;
			internalRedirectCount++;
			goto _skip;
		}

		if (ReqH.empty())
		{
			try						{ setHeaderOrReadMore(); }
			catch (exception& e)	{ throw; }
		}

		if (!ReqH.empty())
		{
			try						{ setBodyOrReadMore(); }
			catch (exception& e)	{ throw; }
		}

_skip:
		method = ReqH.getMethod();
		if (!(method == "GET" || method == "POST" ||
			  method == "PUT" || method == "DELETE"))
				throw methodNotAllowed();	/* for tester */
		if (CHECK->isForbiddenMethod(this->conf, method))
			throw forbidden();
		if (conf->d_return.first)
			CHECK->externalRedirect(conf, ReqH["Host"], linkServerSock->getPort(), serverName);

		uriPath = this->ReqH.getURI().path;

		try 						{ filename = CHECK->getFileName(this->conf, uriPath);}
		catch (HTTP_Error& e)		{ throw; }

		if (method == "POST")
			if (isDynamicResource(this->conf, filename) == false)
				throw methodNotAllowed();
		if (method == "PUT" || method == "DELETE")
		{
			if (this->conf->file_access == false)
			{
				if (isDynamicResource(this->conf, filename) == false)
					throw methodNotAllowed();
			}
			else
			{
				pair<status_code_t, string> ret;
				try						{ ret = controlFile(method, this, filename); }
				catch (HTTP_Error& e)	{ throw; }

				this->ResH.setStatusCode(ret.first);
				this->ResH.setReasonPhrase(ret.second);
				if (ResH.getStatusCode() == 201)
				{
					this->ResH["Content-Length"] = "0";
					this->ResH["Location"] = "http://" + this->ReqH["Host"] + uriPath;
				}
				this->makeResponseHeader();
				if (method == "DELETE") POLLSET->prepareSend(this);
				return;
			}
		}

		try							{ s =_checkFile(filename);
									  if (S_ISDIR(s.st_mode) && filename[filename.length() - 1] != '/')
									  		throw movedPermanently("http://" + this->ReqH["Host"] + uriPath + '/');
									}
		catch (HTTP_Error& e)		{
									  throw;
									}

		try							{ filename = checkIndex(this->conf, filename); }
		catch (HTTP_Error& e)		{ throw; }
		catch (autoIndex& a)		{ this->ResH.setStatusCode(200);
									  this->ResB.setContent(directoryListing(a.path, CHECK->routeRequestTarget(conf, uriPath).first));	//FIXIT: prefix
									  this->ResH["Content-Length"]	= toString(this->ResB.getContent().length());
									  this->ResH["Content-Type"]	= "text/html";
									  this->makeResponseHeader();
									  throw ;
									}

		POLLSET->unsetEvent(this, POLLIN);	/* DO NOT ALLOW RECV UNTIL READ FROM OTHER STREAM DONE */
		string executable = CHECK->getCGIexcutable(this->conf, "." + getExt(filename));
		if ( executable.empty() == false )		// if ".py" is directory, we don't run CGI
		{
			this->ResB.clear();
			this->ResH.removeKey("content-length");
			if (this->linkInputPipe == NULL)
				return createCGI(this->linkServerSock, this, executable, filename);
		}
		createInputFileStream(this, filename);	//readMore
	}




	void	ConnSocket::unlink(Stream* link)	//NOTE: unlink each side?
	{
		if (linkInputFile == link)
		{
			linkInputFile->linkConn = NULL;
			linkInputFile = NULL;
		}
		else if (linkOutputFile == link)
		{
			linkOutputFile->linkConn = NULL;
			linkOutputFile = NULL;
		}
		else if (linkInputPipe == link)
		{
			linkInputPipe->linkConn = NULL;
			linkInputPipe = NULL;
		}
		else if (linkOutputPipe == link)
		{
			linkOutputPipe->linkConn = NULL;
			linkOutputPipe = NULL;
		}
	}

	void	ConnSocket::unlinkAll()	//NOTE: unlink each side?
	{
		if (linkInputFile)
		{
			linkInputFile->linkConn = NULL;
			linkInputFile = NULL;
		}
		if (linkOutputFile)
		{
			linkOutputFile->linkConn = NULL;
			linkOutputFile = NULL;
		}
		if (linkInputPipe)
		{
			linkInputPipe->linkConn = NULL;
			linkInputPipe = NULL;
		}
		if (linkOutputPipe)
		{
			linkOutputPipe->linkConn = NULL;
			linkOutputPipe = NULL;
		}
	}

	bool	ConnSocket::isPipeAlive()
	{
		pid_t	pid	= 0;

		pid = waitpid(linkInputPipe->pid, &linkInputPipe->status, WNOHANG);
		if (!(pid == linkInputPipe->pid || pid == -1))
		{
			LOGGING(ConnSocket, _NOTE(pipe still alive: ) UL("%d"), linkInputPipe->pid);
			// LOGGING(, "") <<  _NOTE(pipe still alive: ) <<  _UL << linkInputPipe->pid << _NC << endl;
			return true;
		}
		else
		{
			// LOGGING(ConnSocket) << _GOOD(waitpid on ) << linkInputPipe->pid << CYAN(" returns ") << _UL << pid << _NC << endl;
			LOGGING(ConnSocket, _GOOD(waitpid on ) UL("%d") CYAN(" returns ") UL("%d"), linkInputPipe->pid, pid);
			return false;
		}
	}

	void	ConnSocket::gracefulClose()
	{
		//NOTE: what if client doesn't send FIN? now we send FIN and close "after client send FIN too".
		// Have to close() instantly after send FIN, with short timer.
		shutdown(this->fd, SHUT_WR);
		/*
			IMPL: if lingering_time, last_active = now, this->setTimeOut(time_t to)
			need to move FIN sended, we do not any processing
		*/

		/*
			for case that client keep sending message even after FIN.
			we prevent calling ConnSocket#send().
		*/
		FINsended = true;

		LOGGING(ConnSocket, _GOOD(server send FIN: ) UL("%d"), this->fd);
	}

	void	ConnSocket::setHeaderOrReadMore()
	{
		if (has2CRLF(recvContent))	//NOTE: what if bad-format request doesn't contain "\r\n\r\n"?
		{
			//IMPL: keep-alive request count--
			if (isValidHeader(recvContent, "\r\n", true))
			{
				currentReqCount++;
				/* set ReqH here */
				ReqH.setMethod(extractFirstWord(recvContent));
				ReqH.setRequestTarget(extractFirstWord(recvContent));
				ReqH.setHTTPversion(extractFirstWord(recvContent));
				ReqH.setURI(splitRequestTarget(ReqH.getRequestTarget()));
				ReqH.setContent(extractHeader(recvContent));
				ReqH.setHeaderField(KVtoMap(this->ReqH.getContent(), ':'));
#ifdef PRINTHEADER
				cout << RED("<-----------------") << endl;
				cout << this->ReqH.getMethod() << " " << this->ReqH.getRequestTarget() << " " << this->ReqH.getHTTPversion() << endl;
				cout << this->ReqH.getContent() << endl;
				cout << RED("<-----------------") << endl;
#endif
				/* conf is still default_server conf */
				if (ReqH.exist("Host"))
				{
					/* find server_names matched with Host */
					this->conf = CONF->getMatchedServer(this->linkServerSock, ReqH["Host"]);
					this->serverName = CONF->getServerName(this->linkServerSock,ReqH["Host"]);
					/* find location matched with URI, or keep server config */
					this->conf = CONF->getMatchedLocation(ReqH.getURI().path,
														  CONVERT(this->conf, ServerConfig));
				}
				else
					throw badRequest();

				if (currentReqCount == 1)
				{
					if (
						(ReqH.exist("Connection") && ReqH["Connection"] == "close") ||
						// this->conf->keepalive_timeout == 0 ||
						this->conf->keepalive_requests == 0
					)
					this->keepAlive = false; /* default: true */
				}

				/* extract trailing body */
				// recvContent = pickOutBody(recvContent);
			}
			else
				throw badRequest();
		}
		else
			throw readMore();
	}

	void	ConnSocket::setBodyOrReadMore()
	{
		string method = ReqH.getMethod();
		if (ReqH.exist("Transfer-Encoding"))
		{
			if (ReqH.exist("Content-Length"))
				throw badRequest();
			if (ReqH["Transfer-Encoding"] == "chunked")
			{
				ReqB.setChunk(recvContent);
				try
				{
					ReqB.decodingChunk(conf->client_max_body_size);
					recvContent = ReqB.chunk.trailingData;	/* if not ALL DONE, it throws exception */
				}
				catch (exception& e)
				{
					if (CONVERT(&e, ReqBody::invalidChunk))
						throw badRequest();
					if (CONVERT(&e, ReqBody::limitExeeded))
						throw payloadTooLarge();
					if (CONVERT(&e, readMore))
						throw readMore();
				}
			}
			else
				throw notImplemented();
		}
		else if (ReqH.exist("Content-Length"))
		{
			if (!isNumber(ReqH["Content-Length"]))
				throw badRequest();

			unsigned CL = toNum<unsigned int>(ReqH["Content-Length"]);
			if (CL <= recvContent.length())
			{
				if (CL > conf->client_max_body_size)
				{
					ReqB.setContent(recvContent.substr(0, conf->client_max_body_size));
					recvContent = recvContent.substr(conf->client_max_body_size);
					throw payloadTooLarge();
				}
				ReqB.setContent(recvContent.substr(0, CL));
				recvContent = recvContent.substr(CL);
			}
			else
				throw readMore();
			/*
				if timeout before read all content-length,
				send 408(Request timeout), close connection.
			*/
		}
		else
		{
			if (!(method == "GET" || method == "HEAD")
			&&	recvContent.empty() == false)
				throw lengthRequired();
		}
	}

	void	ConnSocket::recv()
	{
		ssize_t		byte	= 0;
		switch (byte = readFrom(this->fd, this->recvContent))
		{
		case 0:

			LOGGING(ConnSocket, GRAY("CLIENT EXIT ") UL("%d"), this->fd);
			if (ReqH.exist("Content-Length") &&
				toNum<unsigned int>(ReqH["Content-Length"]) > ReqB.getContent().length())
			{
				/*
					if connection closed before get all content-length,
					send 400(Bad request), close connection.
				*/
				throw badRequest();
			}
			throw connClosed();
			break;

		case -1:
			LOGGING(ConnSocket, YELLOW("SOMETHING WRONG"));
			// throw somethingWrong(strerror(errno));
			throw somethingWrong();
			break;
		default:

			;
		}
	}

	void	ConnSocket::coreDone()
	{
		if (this->linkOutputFile)	/*  <------ output file is just created, do not send, do not close. */
			return ;

		// POLLSET->prepareSend( this );	// set POLLOUT only if autodir ?
	}

	void	ConnSocket::send(const string& content)
	{
		if (FINsended) return;
		try						{ UNDONEBUF->at(this->fd); }
		catch (exception& e)	{ (*UNDONEBUF)[this->fd] = (struct undone){"",0};
								  (*UNDONEBUF)[this->fd].content.append(content.data(), content.length());	}

		string&		rContent	= (*UNDONEBUF)[this->fd].content;
		ssize_t&	rWrited		= (*UNDONEBUF)[this->fd].totalWrited;
		ssize_t		rContentLen	= rContent.length();
		ssize_t		byte		= 0;
		byte = write( this->fd,
					  rContent.data() + rWrited,
					  rContentLen - rWrited );

		ResH.clear(), ResB.clear();
		if (byte >= 0)
			rWrited += byte;
		else
		{
			LOGGING(ConnSocket, _FAIL(unexpected error: ) "%d", errno);
			UNDONEBUF->erase(this->fd);

			this->unlinkAll();
			gracefulClose();	/* maybe drop after get FIN from client */

			throw exception();	// close and Drop now!
		}

		//@ all data sended @//
		if (rWrited == rContentLen)
		{
			LOGGING(ConnSocket, _GOOD(all data sended to )  "%d: %zu / %zu bytes", this->fd, rWrited, rContentLen);
			UNDONEBUF->erase(this->fd);
			if (linkInputPipe && linkInputPipe->readDone == false)
				throw readMore();	/* it was chunked data from pipe */


			this->unlinkAll();
			if (this->keepAlive == false)
				return gracefulClose();	/* maybe drop after get FIN from client */

			ReqH.clear(), ReqB.clear();
			this->chunk = false;
			this->pending = true;
			this->internalRedirectCount = 0;
			if (!recvContent.empty())
			{
				throw gotoCore();
			}
		}

		//' not all data sended. have to be buffered '//
		else
		{
			LOGGING(ConnSocket, _NOTE(Not all data sended to)  "%d: %zu / %zu bytes", this->fd, rWrited, rContentLen);
			throw sendMore();
		}
	}


	void	ConnSocket::setErrorPage(status_code_t status, const string& reason, const string& text)
	{
		this->ResH.setStatusCode(status);
		this->ResH.setReasonPhrase(reason);
		this->ResB.setContent(
							errorpage(
									toString(status) + " " + reason,
									reason,
									text
								)
							);
		this->ResH["Content-type"] = "text/html; charset=iso-8859-1";
		this->ResH["Content-Length"] = toString(this->ResB.getContent().length());
	}

	void	ConnSocket::dummy() {}

	void	ConnSocket::makeResponseHeader()
	{
		this->ResH.clearContent();	/* keep headerfield, remove content only */

		if (this->keepAlive == true && this->currentReqCount < this->conf->keepalive_requests)
			this->ResH["Connection"] = "keep-alive";
		else
		{
			this->ResH["Connection"] = "close";
			this->keepAlive = false;
		}

		this->ResH.setHTTPversion("HTTP/1.1");
		this->ResH.fetchStatusField();
		this->ResH.setDefaultHeaders();
		this->ResH.makeStatusLine();
		this->ResH.integrate();
#ifdef PRINTHEADER
		cout << CYAN("----------------->") << endl;
		cout << this->ResH.getContent();
		cout << CYAN("----------------->") << endl;
#endif
	}

	string		ConnSocket::getOutputContent() { return this->ResH.getContent() + this->ResB.getContent(); }

void	ConnSocket::returnError(HTTP_Error& error)
{
	HTTP_3XX_Error* r =  CONVERT(&error, HTTP_3XX_Error);
	this->ResH.clear();
	this->ResB.clear();
	if (r)
		this->ResH["Location"] = r->location;
	if (CONVERT(&error, HTTP_5XX_Error)
	||	CONVERT(&error, badRequest)				/* 400 */
	||	CONVERT(&error, lengthRequired)			/* 411 */
	||	CONVERT(&error, payloadTooLarge)		/* 413 */
	||	CONVERT(&error, URITooLong)				/* 414 */
	)
		keepAlive = false;
	this->setErrorPage(error.status, error.what(), error.what());
	this->makeResponseHeader();
}

void ConnSocket::checkErrorPage()
{
	typedef pair<int, string> 		_ERRORPAIR;
	typedef map<int, _ERRORPAIR>	_ERRORMAP;

	/* error_page not supported for CGI-documentResponse, CGI-clientRedir */

	if (this->pending == false)	return;

	status_code_t 	status = this->ResH.getStatusCode();
	_ERRORMAP 	error_page = this->conf->error_page;
	_ERRORMAP::iterator	it = error_page.find(status);

	if (it == error_page.end())				return;
	if (this->internalRedirectCount == MAX_INTERNAL_REDIRECT)	{ throw internalServerError(); }

	_ERRORPAIR	status_URI = (it->second);

	try								{ 
									  this->ResH.setStatusCode(status_URI.first);
									  this->ReqH.setRequestTarget(status_URI.second);
									  this->ReqH.setURI(splitRequestTarget(this->ReqH.getRequestTarget()));
									}
	catch (HTTP_Error& e)			{ throw internalServerError(); }

#ifdef PRINTHEADER
	cout << YELLOW("ERRORPAGE REDIR TO: ")  << ReqH.getURI().path << endl;
#endif
	this->ReqH.setMethod("GET");
	this->conf = CONF->getMatchedServer(this->linkServerSock, this->ReqH["Host"]);
	this->serverName = CONF->getServerName(this->linkServerSock,ReqH["Host"]);
	this->conf = CONF->getMatchedLocation(this->ReqH.getURI().path,
										CONVERT(this->conf, ServerConfig));
	// this->unlinkAll();
	throw ::internalRedirect();
}

/**========================================================================
* !                            Exceptions
*========================================================================**/

ConnSocket::connClosed::connClosed(): msg("") {}
ConnSocket::connClosed::connClosed(const string& m): msg(m) {}
ConnSocket::connClosed::~connClosed() throw() {};
const char *	ConnSocket::connClosed::what() const throw() { return msg.c_str(); }

/**========================================================================
* ,                               Others
*========================================================================**/

pair<status_code_t, string>
controlFile(const string& method, ConnSocket* connected, const string& filename)
{
	if		(method == "PUT")		return createPUToutputFile(connected, filename);
	else if	(method == "DELETE")	return deleteFile(filename);

	throw internalServerError();
}

pair<status_code_t, string>
createPUToutputFile(ConnSocket* connected, const string filename)
{

	FileStream* f = new FileStream(filename);

	bool alreadyExist = (access(filename.c_str(),F_OK) == 0);

	f->open(O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK, 0777);
	if (f->getFD() == -1)
		throw Conflict();

	connected->linkOutputFile = f;
	f->linkConn = connected;

	POLLSET->enroll(f, POLLOUT);

	return alreadyExist ?  make_pair<status_code_t>(204, "No Content") :
						   make_pair<status_code_t>(201, "Created" );

}

pair<status_code_t, string>
deleteFile(const string& filename)
{
	struct 	stat s;

	try						{ s = _checkFile(filename); }
	catch (HTTP_Error& e)	{ throw; }

	if (S_ISDIR(s.st_mode) && filename[filename.length() - 1] != '/')
		throw Conflict();

	if (remove(filename.c_str()) == 0)
		return make_pair<status_code_t, string>(204, "No Content");
	throw Conflict();
}

bool	isCGI(Config* conf, const string& filename)
{
	string CGIexecutable;

	CGIexecutable = CHECK->getCGIexcutable(conf, "." + getExt(filename));
	if ( CGIexecutable.empty() == false )
		return true;
	return false;
}
bool	isDynamicResource(Config* conf, const string& filename)
{
	if (isCGI(conf, filename)
		/* ... */
							)
		return true;
	return false;
}
