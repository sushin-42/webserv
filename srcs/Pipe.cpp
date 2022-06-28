#include "Pipe.hpp"
#include "CGI.hpp"
#include "ConfigLoader.hpp"
#include "ConnSocket.hpp"
#include "Exceptions.hpp"

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
				this->output.rfind("\n\n") != string::npos)
		{
			// cout << "GOT HEADER!" << endl;
			this->processOutputHeader();
			connected->pending = false;
			this->headerDone = true;
			this->output = extractBody(this->output);	// store remained after header
			if (this->output.empty())
				return ;		 						// if extracted trailing Body == '', makeChunk will send '0\r\n\r\n'
		}
		else
		{
			// cout << "WAIT HEADER..." << endl;
			throw readMore();
		}
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
		TAG(CGI#, CGIroutines); cout << RED("Unexcpected error from pipe: ") << this->getFD() << endl;
		this->close();
		throw internalServerError();

	case 0:		/* close pipe, process output */

		TAG(CGI#, CGIroutines); cout << GRAY("Pipe closed: ") << this->getFD() << endl;
		this->close();
		this->readDone = true;
		connected->pending = false;
		break;

	default:
		break;
	}
}

void	Pipe::coreDone()
{
	ConnSocket* connected = this->linkConn;

	// if (headerDone)
	if (connected->pending == false)
		POLLSET->prepareSend( connected );
	return;
}

string	Pipe::getOutputContent() { return this->linkConn->ReqB.getContent();  }
void	Pipe::send(const string& content, map<int, struct undone>& writeUndoneBuf)
{
	try						{ writeUndoneBuf.at(this->fd); }
	catch (exception& e)	{ writeUndoneBuf[this->fd] = (struct undone){"",0};
								writeUndoneBuf[this->fd].content.append(content.data(), content.length());	}

	string&		rContent	= writeUndoneBuf[this->fd].content;
	ssize_t&	rWrited		= writeUndoneBuf[this->fd].totalWrited;
	ssize_t		rContentLen	= rContent.length();
	ssize_t		byte		= 0;

	byte = ::write( this->fd,
					rContent.data() + rWrited,
				//   1);
					rContentLen - rWrited );
	if (byte > 0)
		rWrited += byte;

	//@ all data sended @//
	if (rWrited == rContentLen)
	{
		TAG(Pipe, send) << _GOOD(all data sended to child process) << this->fd << ": " << rWrited << " / " << rContentLen << " bytes" << endl;
		writeUndoneBuf.erase(this->fd);
		this->linkConn->linkOutputPipe = NULL;
		close();
		throw exception();
	}
	//' not all data sended. have to be buffered '//
	else
	{
		TAG(Pipe, send) << _NOTE(Not all data sended to) << this->fd << ": " << rWrited << " / " << rContentLen  << " bytes" << endl;
		if (byte == -1)
		{
			TAG(Pipe, send) << _FAIL(unexpected error: ) << errno << endl;
			writeUndoneBuf.erase(this->fd);
			close();
			throw exception();	// close and Drop now!
								// NOTE: need to drop linkConn?
		}
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
		connected->ReqH.setRequsetTarget(connected->ResH["location"]);
		connected->ResH.removeKey("location");
		connected->ResH.removeKey("transfer-encoding");

		connected->conf = CONF->getMatchedServer(connected->linkServerSock, connected->ReqH["Host"]);
		connected->conf = CONF->getMatchedLocation(connected->ReqH.getRequsetTarget(),
													CONVERT(connected->conf, ServerConfig));

		cout << RED("LOCAL REDIR TO: ")  << connected->ReqH.getRequsetTarget() << endl;

		this->close();
		POLLSET->drop(this);
		connected->unlink(this);

		if (connected->internalRedirectCount == 0)
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

	cout  << "DOC" << endl;
	if (!connected->ResH.exist("Content-Length") &&
			(!connected->ResH.exist("Transfer-Encoding") ||
			lowerize(connected->ResH["Transfer-encoding"]) != "chunked"
		)
	)
		setChunkEncoding();
	connected->makeResponseHeader();
}

