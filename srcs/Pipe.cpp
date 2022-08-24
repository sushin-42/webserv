#include "Pipe.hpp"
#include "CGI.hpp"
#include "ConfigLoader.hpp"
#include "ConnSocket.hpp"
#include "Exceptions.hpp"
#include "HTTP_Error.hpp"
#include "WriteUndoneBuf.hpp"
#include "utils.hpp"

/**========================================================================
* @                           Constructors
*========================================================================**/

	Pipe::Pipe()
	: Stream(-1), output(), pid(0), status(0), headerDone(false), readDone(false), linkConn(NULL) {}

	Pipe::Pipe( int fd, pid_t p )
	:Stream(fd), output(), pid(p), status(0), headerDone(false), readDone(false), linkConn(NULL) {}

	Pipe::Pipe( const Pipe& src )
	: Stream(src), output(src.output), pid(src.pid), status(src.status), headerDone(false), readDone(false), linkConn(src.linkConn) {}

	Pipe::~Pipe() {}

/**========================================================================
* *                            operators
*========================================================================**/

	Pipe&	Pipe::operator=( const Pipe& src )
	{
		if (this != &src)
		{
			this->Stream::operator=(src);
			this->output 	= src.output;
			this->pid	 	= src.pid;
			this->status 	= src.status;
			this->headerDone= src.headerDone;
			this->linkConn	= src.linkConn;
		}
		return *this;
	}

/**========================================================================
* #                          member functions
*========================================================================**/

void	Pipe::core()
{
	ConnSocket* connected = this->linkConn;

	if (this->headerDone == false)
	{
		connected->pending = true;
		if	(this->output.rfind("\r\n\r\n") != string::npos ||
			 this->output.rfind("\n\n") != string::npos		||
			 readDone)
		{
			this->processOutputHeader();
			connected->pending = false;
			this->headerDone = true;
			this->output = pickOutBody(this->output);	// store remained after header
			if (this->output.empty())
				return ;		 						// if extracted trailing Body == '', makeChunk will send '0\r\n\r\n'
		}
		else
			throw readMore();
	}
	/* header done, body part exists */

	if (connected->pending == false)
	{
		// cout << "OUTPUT IS..." << endl;
		connected->ResB.setContent(
									connected->chunk ?
										makeChunk(this->output) :
										this->output
								);
		this->output.clear();
	}
}

void	Pipe::recv()
{
	ConnSocket* connected = this->linkConn;

	ssize_t	byte = 0;

	switch (byte = readFrom(this->fd, this->output))
	{

	case -1:	/* internal server error */
		LOGGING(Pipe,  RED("Unexcpected error from pipe: ") UL("%d"), this->getFD());

		linkConn->unlink(this);
		POLLSET->drop(this);
		POLLSET->setEvent(connected, POLLIN);
		throw internalServerError();

	case 0:		/* close pipe, process output */
		LOGGING(Pipe,  GRAY("pipe closed: ") UL("%d"), this->getFD());

		this->readDone = true;
		connected->pending = false;
		POLLSET->setEvent(connected, POLLIN);
		break;

	default:
		break;
	}
}

void    Pipe::coreDone()
{
	ConnSocket* connected = this->linkConn;

	if (connected->pending == false)
	{
		POLLSET->prepareSend( connected );
	}
	if (readDone)
	{
		connected->pending=true;
		connected->unlink(this);
		POLLSET->drop(this);
	}
	return;
}

string	Pipe::getOutputContent() { return this->linkConn->ReqB.getContent();  }
void	Pipe::send(const string& content)
{

	try						{ UNDONEBUF->at(this->fd); }
	catch (exception& e)	{ (*UNDONEBUF)[this->fd] = (struct undone){"",0};
							  (*UNDONEBUF)[this->fd].content.append(content.data(), content.length());	}
	string&		rContent	= (*UNDONEBUF)[this->fd].content;
	ssize_t&	rWrited		= (*UNDONEBUF)[this->fd].totalWrited;
	ssize_t		rContentLen	= rContent.length();
	ssize_t		byte		= 0;


	errno = 0;
	byte = ::write( this->fd,
					rContent.data() + rWrited,
					rContentLen - rWrited );

	if (byte >= 0)
		rWrited += byte;
	else
	{
		LOGGING(Pipe, _FAIL(unexpected error: ) "%d", errno);
		UNDONEBUF->erase(this->fd);

		linkConn->unlink(this);
		POLLSET->drop(this);

		throw exception();	// close and Drop now!
							// NOTE: need to drop linkConn?
	}


	//@ all data sended @//
	if (rWrited == rContentLen)
	{
		LOGGING(Pipe, _GOOD(all data writed to pipe )  UL("%d") ": %zu / %zu bytes", this->getFD(), rWrited, rContentLen);
		UNDONEBUF->erase(this->fd);

		linkConn->unlink(this);
		POLLSET->drop(this);
	}
	//' not all data sended. have to be buffered '//
	else
	{
		LOGGING(Pipe, _NOTE(Not all data sended to )  UL("%d") ": %zu / %zu bytes", this->fd, rWrited, rContentLen);

		throw sendMore();
	}
}


void	Pipe::moveToResH(const string& output)
{
	ConnSocket* connected = this->linkConn;
	if (!connected)	return ;

	map<string,string>				tmp(KVtoMap(output, ':'));
	map<string,string>::iterator	it, ite;

	it = tmp.begin(), ite = tmp.end();
	for (; it != ite; it++)
	{
		if (it->first == lowerize("Content-type")||
			it->first == lowerize("Status")||
			it->first == lowerize("Location")||
			it->first == lowerize("Content-Length")||
			it->first == lowerize("Content-Range")||
			// it->first == lowerize("Transfer-Encoding")||
			it->first == lowerize("ETag")||
			connected->ResH.exist(it->first) == false	)

				connected->ResH[it->first] = it->second;
		else
				connected->ResH.append(it->first, it->second);
	}
}

void	Pipe::setChunkEncoding()
{
	ConnSocket* connected = this->linkConn;
	if (!connected)	return ;


	{
		connected->ResH.append("Transfer-encoding",  "chunked");
		connected->chunk = true;
		/**========================================================================
		 *'  			Server chunk. NOT same with script chunk
		 *	we encode(chunk) if and only if we set "Transfer-Encoding: chunked".
		 *	we DO NOT encode even if "Transfer-Encoding: chunked" exists in script output.
		 *	if we get output with it, body of the output will be send raw.
		 *	if script wants to be chunked, it MUST be chunked format by itself.
		 *
		 *========================================================================**/
	}
}

void	Pipe::processOutputHeader()
{
	ConnSocket* connected = this->linkConn;
	if (!connected)	return ;

	pair<status_code_t, string>		Status;

	if (isValidHeader(pickOutHeader(this->output, "\r\n"), "\r\n", true) == false &&
		isValidHeader(pickOutHeader(this->output, "\r\n"), "\r\n", false) == false &&
		isValidHeader(pickOutHeader(this->output, "\n"),   "\n",   true) == false &&
		isValidHeader(pickOutHeader(this->output, "\n"),   "\n",   false) == false)
			throw internalServerError();
			

	moveToResH(this->output);

	if (connected->ResH.exist("Status") == false)
		connected->ResH["Status"] = "200 OK";

	if (connected->ResH.exist("Location"))
	{
		if (connected->ResH["Location"][0] == '/')	localRedir();
		else										clientRedir();
	}
	else											documentResponse();


}

//*--------------------------------------------------------------------------*//
//* local-redir-response = local-Location NL                                 *//
//*                                                                          *//
//*  The CGI script can return a URI path and query-string                   *//
//*	 ('local-pathquery') for a local resource in a Location header field.    *//
//*  This indicates to the server that it should reprocess the request       *//
//*  using the path specified.                                               *//
//*                                                                          *//
//*  The script MUST NOT return any other header fields or a message-body,   *//
//*  and the server MUST generate the response that it would have produced   *//
//*  in response to a request containing the URL                             *//
//*                                                                          *//
//*  scheme "://" server-name ":" server-port local-pathquery                *//
//*--------------------------------------------------------------------------*//

void	Pipe::localRedir()
{
	ConnSocket* connected = this->linkConn;
	if (!connected)	return ;

	if (checkStatusField(connected->ResH["Status"]).first == 200)
	{

		//@ regard as request to Location, but some header-fields from CGI remain @//
		//@ Content-Length, Content-Type, Transfer-Encoding will be replaced @//
		connected->pending = true;
		connected->ReqH.setRequestTarget(connected->ResH["location"]);
		connected->ReqH.setURI(splitRequestTarget(connected->ReqH.getRequestTarget()));
		connected->ResH.removeKey("location");
		connected->ResH.removeKey("transfer-encoding");

		connected->conf = CONF->getMatchedServer(connected->linkServerSock, connected->ReqH["Host"]);
		connected->serverName = CONF->getServerName(connected->linkServerSock,connected->ReqH["Host"]);
		connected->conf = CONF->getMatchedLocation(connected->ReqH.getURI().path,
													CONVERT(connected->conf, ServerConfig));

		

#ifdef PRINTHEADER
	cout << PURPLE("CGI LOCAL REDIR TO: ")  << connected->ReqH.getURI().path << endl;
#endif

		connected->unlink(this);
		POLLSET->drop(this);

		if (connected->internalRedirectCount == MAX_INTERNAL_REDIRECT)
			throw internalServerError();
		else
			throw internalRedirect();	/* goto connected->core phase, deligate makeResponseHeader to FileStream. */
	}
	else
		connected->makeResponseHeader();	/* NOTE: if not 200? apache */

}


//'--------------------------------------------------------------------------'//
//' client-redir-response = client-Location *extension-field NL              '//
//'                                                                          '//
//'   The CGI script can return an absolute URI path in a Location header    '//
//'	 field, to indicate to the client that it should reprocess the request   '//
//'  using the URI specified.                                                '//
//'                                                                          '//
//'   The script MUST not provide any other header fields, except for        '//
//'  server-defined CGI extension fields.  For an HTTP client request, the   '//
//'  server MUST generate a 302 'Found' HTTP response message.               '//
//'--------------------------------------------------------------------------'//

//%--------------------------------------------------------------------------%//
//% client-redirdoc-response = client-Location Status Content-Type           %//
//%                            *other-field NL response-body                 %//
//%                                                                          %//
//%   The CGI script can return an absolute URI path in a Location header    %//
//%	 field together with an attached document, to indicate to the client     %//
//%  that it should reprocess the request using the URI specified.           %//
//%                                                                          %//
//%   The Status header field MUST be supplied and MUST contain a status     %//
//%  value of 302 'Found', or it MAY contain an extension-code, that is,     %//
//%  another valid status code that means client redirection.  The server    %//
//%  MUST make any appropriate modifications to the script's output to       %//
//%  ensure that the response to the client complies with the response       %//
//%  protocol version.                                                       %//
//%--------------------------------------------------------------------------%//

void	Pipe::clientRedir()		//check 303
{
	ConnSocket* connected = this->linkConn;
	if (!connected)	return ;

	if (checkStatusField(connected->ResH["Status"]).first == 200)
	{
		connected->ResH["Status"] = "302 Found";
		connected->pending = true;
		connected->ResB.setContent(
						errorpage(
								"302 Found",
								"Found",
								"<p>The document has moved <a href=\"" + connected->ResH["location"] + "\">here</a>.</p>"
							)
						);
		connected->ResH["Content-type"] = "text/html; charset=iso-8859-1";
		connected->ResH["Content-Length"] = toString(connected->ResB.getContent().length());
		connected->ResH.removeKey("Transfer-Encoding");
		connected->makeResponseHeader();
	}

	else
		connected->makeResponseHeader();	/* NOTE: if not 200? apache */
		/*NOTE: if Not 200? APACHE*/

	// if (ResH.getStatusCode() != 206 || ResH.getStatusCode() != 416)
		// erase Content-Range
}


//.--------------------------------------------------------------------------.//
//. document-response = Content-Type [ Status ] *other-field NL              .//
//.                          response-body                                   .//
//.                                                                          .//
//.  The script MUST return a Content-Type header field.                     .//
//.	 A Status header field is optional, and status 200 'OK' if omitted.      .//
//.  The server MUST make any appropriate modifications to the script output .//
//.  to ensure that the response to the client                               .//
//.  complies with the response protocol version.                            .//
//.--------------------------------------------------------------------------.//

void	Pipe::documentResponse()
{
	ConnSocket* connected = this->linkConn;
	if (!connected)	return ;

	if (!connected->ResH.exist("Content-Length") &&
			(!connected->ResH.exist("Transfer-Encoding") ||
			lowerize(connected->ResH["Transfer-encoding"]) != "chunked"
		)
	)
		setChunkEncoding();
	connected->makeResponseHeader();
}

