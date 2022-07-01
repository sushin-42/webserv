# include "ConnSocket.hpp"
# include "CGI.hpp"
# include "Exceptions.hpp"
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
	  linkServerSock(NULL), conf(NULL) {}

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
		struct stat s;
		string			reqTarget;

		string			filename;
		string			ext;

		bool			alreadyExist = false;

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

		if (CHECK->isAllowed(this->conf, ReqH.getMethod()) == false)
			throw methodNotAllowed();

		// recvContent.clear();
		reqTarget = this->ReqH.getRequsetTarget();

		try 						{ filename = CHECK->getFileName(this->conf, reqTarget); }
		catch (httpError& e)		{ throw; }


		if (this->ReqH.getMethod() == "PUT")
		{
			try						{ alreadyExist = createPUToutputFile(this, filename); (void)alreadyExist; }
			catch (Conflict& e)		{ throw; }

			// alreadyExist ? throw noContent() : throw Created();
			this->ResH.setStatusCode(alreadyExist ? 204 : 201);
			this->ResH.setReasonPhrase(alreadyExist ? "No Content" : "Created");
			this->makeResponseHeader();

			return;
		}

		try							{ s =_checkFile(filename);
									  if (S_ISDIR(s.st_mode) && filename.back() != '/')
									  		throw movedPermanently("http://" + this->ReqH["Host"] + reqTarget + '/');
									}
		catch (httpError& e)		{
									  throw;
									}

		try							{ filename = checkIndex(this->conf, filename); }
		catch (httpError& e)		{ throw; }
		catch (autoIndex& a)		{ this->ResH.setStatusCode(200);
									  this->ResB.setContent(directoryListing(a.path, "/"));	//FIXIT: prefix
									  this->ResH["Content-Length"]	= toString(this->ResB.getContent().length());
									  this->ResH["Content-Type"]	= "text/html";
									  this->makeResponseHeader();
									  throw ;
									}

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
			if (isValidHeader(recvContent))
			{
				currentReqCount++;
				/* set ReqH here */
				ReqH.setMethod(checkMethod(recvContent));
				ReqH.setHTTPversion("HTTP/1.1");	//TODO: parse from request
				ReqH.setRequsetTarget(recvContent);
				ReqH.setContent(extractHeader(recvContent));
				ReqH.setHeaderField(KVtoMap(recvContent, ':'));
#ifdef PRINTHEADER
				cout << RED("<-----------------") << endl;
				cout << this->ReqH.getContent();
				cout << RED("<-----------------") << endl;
#endif
				/* conf is still default_server conf */
				if (ReqH.exist("Host"))
				{
					/* find server_name matched with Host */
					this->conf = CONF->getMatchedServer(this->linkServerSock, ReqH["Host"]);

					/* find location matched with URI, or keep server config */
					this->conf = CONF->getMatchedLocation(ReqH.getRequsetTarget(),
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
				recvContent = extractBody(recvContent);
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
		// if (method == "GET" || method == "HEAD")
			// return;

		if (ReqH.exist("Transfer-Encoding"))	// it will override Content-Length
		{
			/*
				current:
				- discard after "0\r\n\r\n" ?
				- accept payload contains "0\r\n\r\n"
			*/
			ReqB.setChunk(recvContent);
			try
			{
				ReqB.decodingChunk(conf->client_max_body_size);
				recvContent = ReqB.chunk.trailingData;	/* if not ALL DONE, it throws exception */
			}
			catch (exception& e)
			{
				if (CONVERT(&e, ReqBody::invalidChunk))
				{
					cout << "INVALID CHUNK" << endl;
					throw badRequest();
				}
				if (CONVERT(&e, ReqBody::limitExeeded))
					throw payloadTooLarge();
				if (CONVERT(&e, readMore))
					throw readMore();
			}
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
			if (recvContent.empty() == false)
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
			LOGGING(ConnSocket, YELLOW("No data to read"));
			throw somethingWrong(strerror(errno));
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

	void	ConnSocket::send(const string& content, map<int, undone>& writeUndoneBuf)
	{
		if (FINsended) return;
		try						{ writeUndoneBuf.at(this->fd); }
		catch (exception& e)	{ writeUndoneBuf[this->fd] = (struct undone){"",0};
								  writeUndoneBuf[this->fd].content.append(content.data(), content.length());	}

		string&		rContent	= writeUndoneBuf[this->fd].content;
		ssize_t&	rWrited		= writeUndoneBuf[this->fd].totalWrited;
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
			writeUndoneBuf.erase(this->fd);

			this->unlinkAll();
			gracefulClose();	/* maybe drop after get FIN from client */

			throw exception();	// close and Drop now!
		}

		//@ all data sended @//
		if (rWrited == rContentLen)
		{
			LOGGING(ConnSocket, _GOOD(all data sended to )  "%d: %zu / %zu bytes", this->fd, rWrited, rContentLen);
			writeUndoneBuf.erase(this->fd);
			if (linkInputPipe && linkInputPipe->readDone == false)
				throw readMore();	/* it was chunked data from pipe */


			this->unlinkAll();
			if (this->keepAlive == false)
				gracefulClose();	/* maybe drop after get FIN from client */

			ReqH.clear(), ReqB.clear();
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

string checkMethod(const string& content)
{
	string::size_type end = content.find(" ");
	string method = content.substr(0, end);
	return method;
}

// void	ConnSocket::returnError(status_code_t status, const string& message)
void	ConnSocket::returnError(httpError& error)
{
	redirectError* r =  CONVERT(&error, redirectError);
	this->ResH.clear();
	this->ResB.clear();
	if (r)
		this->ResH["Location"] = r->location;
	if (CONVERT(&error, badRequest)
	||	CONVERT(&error, internalServerError)
	||	CONVERT(&error, payloadTooLarge))
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
	if (this->internalRedirectCount == MAX_INTERNAL_REDIRECT)	{throw internalServerError(); }

	_ERRORPAIR	status_URI = (it->second);

	this->ResH.setStatusCode(status_URI.first);
	this->ReqH.setRequsetTarget(status_URI.second);

	cout << CYAN("ERROR REDIR TO: ")  << ReqH.getRequsetTarget() << endl;
	this->ReqH.setMethod("GET");
	this->conf = CONF->getMatchedServer(this->linkServerSock, this->ReqH["Host"]);
	this->conf = CONF->getMatchedLocation(this->ReqH.getRequsetTarget(),
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

bool	createPUToutputFile(ConnSocket* connected, const string filename)
{

	FileStream* f = new FileStream(filename);

	bool alreadyExist = (access(filename.c_str(),F_OK) == 0);

	f->open(O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK);
	if (f->getFD() == -1)
		throw Conflict();

	connected->linkOutputFile = f;
	f->linkConn = connected;

	POLLSET->enroll(f, POLLOUT);

	return alreadyExist;

}
