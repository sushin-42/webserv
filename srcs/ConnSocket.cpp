# include "ConnSocket.hpp"
# include "CGI.hpp"
# include "Exceptions.hpp"
# include "utils.hpp"
# include "ServerConfig.hpp"
# include "ConfigLoader.hpp"
# include "ConfigChecker.hpp"
# include "checkFile.hpp"
#include <exception>
# include <string>
# include "Poll.hpp"




/**========================================================================
* @                           Constructors
*========================================================================**/

	ConnSocket::ConnSocket()
	: ISocket(), len(sizeof(info)),
	  recvContent(), ReqH(), ReqB(), ResH(), ResB(),
	  pending(true), chunk(false), FINsended(false), internalRedirect(false),
	  internalRedirectCount(10),
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
			this->ResH.setDefaultHeaders();
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
									  this->ResH.setDefaultHeaders();					//FIXIT: prefix
									  this->ResB.setContent(directoryListing(a.path, "/"));	/* alias case: need to append Loc URI || or req Target ? */
									  this->ResH["Content-Length"]	= toString(this->ResB.getContent().length());
									  this->ResH["Content-Type"]		= "text/html";
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
			TAG(ConnSocket) <<  _NOTE(pipe still alive: ) <<  _UL << linkInputPipe->pid << _NC << endl;
			return true;
		}
		else
		{
			TAG(ConnSocket) << _GOOD(waitpid on ) << linkInputPipe->pid << CYAN(" returns ") << _UL << pid << _NC << endl;
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

		TAG(ConnSocket) << _GOOD(server send FIN: ) << _UL << this->fd << _NC << endl;
	}

	void	ConnSocket::setHeaderOrReadMore()
	{
		if (has2CRLF(recvContent))	//NOTE: what if bad-format request doesn't contain "\r\n\r\n"?
		{
			//IMPL: keep-alive request count--
			if (isValidHeader(recvContent))
			{
				/* set ReqH here */
				switch (checkMethod(recvContent))
				{
				case GET:	ReqH.setMethod("GET");		break;
				case PUT:	ReqH.setMethod("PUT");		break;
				case POST:	ReqH.setMethod("POST");		break;
				case DELETE:ReqH.setMethod("DELETE");	break;
				default: throw methodNotAllowed();
				}
				ReqH.setHTTPversion("HTTP/1.1");	//TODO: parse from request
				ReqH.setRequsetTarget(recvContent);
				ReqH.setContent(extractHeader(recvContent));
				ReqH.setHeaderField(KVtoMap(recvContent, ':'));

				cout << RED("<-----------------") << endl;
				cout << this->ReqH.getContent();
				cout << RED("<-----------------") << endl;

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

				if (CHECK->isAllowed(this->conf, ReqH.getMethod()) == false)

					throw methodNotAllowed();

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
					throw payloadTooLarge();		//NOTE: 버리는 데이터 처리 필요?
				}
				ReqB.setContent(recvContent.substr(0, CL));
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

			TAG(ConnSocket); cout << GRAY("CLIENT EXIT ") << this->fd << endl;
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
			TAG(ConnSocket) << YELLOW("No data to read") << endl;
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
			TAG(ConnSocket) << _FAIL(unexpected error: ) << errno << endl;
			writeUndoneBuf.erase(this->fd);

			this->unlinkAll();
			gracefulClose();	/* maybe drop after get FIN from client */

			throw exception();	// close and Drop now!
		}

		//@ all data sended @//
		if (rWrited == rContentLen)
		{
			TAG(ConnSocket) << _GOOD(all data sended to) << this->fd << ": " << rWrited << " / " << rContentLen << " bytes" << endl;
			writeUndoneBuf.erase(this->fd);
			if (linkInputPipe && linkInputPipe->readDone == false)
				throw readMore();

			this->unlinkAll();
			gracefulClose();	/* maybe drop after get FIN from client */
		}

		//' not all data sended. have to be buffered '//
		else
		{
			TAG(ConnSocket) << _NOTE(Not all data sended to) << this->fd << ": " << rWrited << " / " << rContentLen  << " bytes" << endl;
			throw sendMore();
		}
	}


	void	ConnSocket::setErrorPage(status_code_t status, const string& reason, const string& text)
	{
		this->ResH.setHTTPversion("HTTP/1.1");
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
		// if (this->ResH.getHeaderField().empty() == false)
		// {
			this->ResH.clearContent();	/* keep headerfield, remove content only */

			this->ResH.setHTTPversion("HTTP/1.1");
			this->ResH.fetchStatusField();
			this->ResH.setDefaultHeaders();
			this->ResH.makeStatusLine();
			this->ResH.integrate();
			cout << CYAN("----------------->") << endl;
			cout << this->ResH.getContent();
			cout << CYAN("----------------->") << endl;
		// }
	}

	string		ConnSocket::getOutputContent() { return this->ResH.getContent() + this->ResB.getContent(); }

char checkMethod(const string& content)
{
	string::size_type end = content.find(" ");
	string method = content.substr(0, end);
	if		(method == "GET")	return GET;
	else if (method == "PUT")	return PUT;
	else if (method == "POST")	return POST;
	else if (method == "DELETE")return DELETE;
	return 0;
}

// void	ConnSocket::returnError(status_code_t status, const string& message)
void	ConnSocket::returnError(httpError& error)
{
	redirectError* r =  CONVERT(&error, redirectError);
	this->ResH.clear();
	this->ResB.clear();
	this->setErrorPage(error.status, error.what(), error.what());
	if (r)
		this->ResH["Location"] = r->location;
	this->ResH.setDefaultHeaders();
	this->ResH.makeStatusLine();
	this->ResH.integrate();
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
	if (this->internalRedirectCount == 0)	{cout <<"ISE" <<endl;throw internalServerError(); }

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
