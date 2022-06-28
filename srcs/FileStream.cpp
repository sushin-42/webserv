#include "FileStream.hpp"
#include "Exceptions.hpp"
#include "ConfigLoader.hpp"
#include "Poll.hpp"
#include "fcntl.h"

/**========================================================================
* @                           Constructors
*========================================================================**/

FileStream::FileStream() : Stream(-1), filename(), content(), internalRedirectCount(10), linkConn(NULL) {}
FileStream::FileStream( int fd ) : Stream(fd), filename(), content(), internalRedirectCount(10), linkConn(NULL) {}
FileStream::FileStream( const string& filename ) : Stream(-1), filename(filename), content(), internalRedirectCount(10), linkConn(NULL)
{
	// open?
}

FileStream::FileStream( const FileStream& src ) : Stream(src.fd), filename(src.filename), content(src.content),linkConn(src.linkConn) {}
FileStream::~FileStream() {}

/**========================================================================
* *                            operators
*========================================================================**/

FileStream&	FileStream::operator=( const FileStream& src )
{
	if (this != &src)
	{
		this->Stream::operator=(src);
		this->linkConn	= src.linkConn;
	}
	return *this;
}

/**========================================================================
* #                          member functions
*========================================================================**/

int	FileStream::open(int mode)
{
	int ret;
	ret = ::open(this->filename.c_str(), mode);

	this->fd = ret;
	return ret;
}

int	FileStream::open(const string& filename, int mode)
{
	int ret;

	this->filename = filename;
	ret = ::open(filename.c_str(), mode);

	this->fd = ret;
	return ret;
}

void			FileStream::recv()
{
	ssize_t byte;
	switch (byte = readFrom(this->fd, this->content))
	{
	case -1:
		TAG(FileStream#, recv); cout << RED("Unexcpected error from file: ") << this->getFilename() << endl;
		this->close();
		throw internalServerError();

	case 0:		/* close file, process output */
		TAG(FileStream#, recv); cout << GRAY("file closed: ") << this->getFilename() << endl;
		this->close();
		break;

	default:	/* content appended */
		throw readMore();
	}
}

void	FileStream::coreDone()
{
	ConnSocket* connected = this->linkConn;
	// POLLSET->drop(it);
	connected->unlink(this);
	POLLSET->prepareSend( connected );
}


void	FileStream::send(const string& content, map<int, struct undone>& writeUndoneBuf)
{
	if (this->fd == -1) return;

	try						{ writeUndoneBuf.at(this->fd); }
	catch (exception& e)	{ writeUndoneBuf[this->fd] = (struct undone){"",0};
							  writeUndoneBuf[this->fd].content.append(content.data(), content.length());	}

	string&		rContent	= writeUndoneBuf[this->fd].content;
	ssize_t&	rWrited		= writeUndoneBuf[this->fd].totalWrited;
	ssize_t		rContentLen	= rContent.length();
	ssize_t		byte		= 0;

	byte = ::write( this->fd,
					rContent.data() + rWrited,
					rContentLen - rWrited );
	if (byte > 0)
		rWrited += byte;

	//@ all data sended @//
	if (rWrited == rContentLen)
	{
		TAG(FileStream, send) << _GOOD(all data writed to File ) _UL << this->filename << _NC << " : " << rWrited << " / " << rContentLen << " bytes" << endl;
		writeUndoneBuf.erase(this->fd);
		close();
		throw exception();
	}
	//' not all data sended. have to be buffered '//
	else
	{
		TAG(FileStream, send) << _NOTE(Not all data sended to) _UL << this->filename << _NC << " : " << rWrited << " / " << rContentLen  << " bytes" << endl;
		if (byte == -1)
		{
			TAG(FileStream, send) << _FAIL(unexpected error: ) << errno << endl;
			writeUndoneBuf.erase(this->fd);
			close();
			throw exception();	// close and Drop now!
								// NOTE: need to drop linkConn?
		}
		throw sendMore();
	}
}

void			FileStream::core()
{
		ConnSocket* connected;
		connected = this->linkConn;
		string ext		= getExt(connected->ReqH.getRequsetTarget());

		connected->ResB.setContent(this->content);
		connected->ResH["Content-Type"]	= CONF->MIME.find(ext) != CONF->MIME.end() ?
										  CONF->MIME[ext] : connected->conf->default_type;

		if (!connected->ResB.getContent().empty())
			connected->ResH["Content-Length"]	= toString(connected->ResB.getContent().length());

		connected->makeResponseHeader();
}

string			FileStream::getOutputContent() { return this->linkConn->ReqB.getContent();  }
void			FileStream::setFilename( const string& filename) { this->filename =filename;}
const string&	FileStream::getFilename() const	{ return this->filename;}
void			FileStream::dummy() {}

/**========================================================================
* ,                               Others
*========================================================================**/

void			createInputFileStream(ConnSocket* connected, const string& filename)
{
	FileStream* f = new FileStream(filename);
	f->open(O_RDONLY);

	connected->linkInputFile = f;
	f->linkConn = connected;
	POLLSET->enroll(f, POLLIN);

	throw readMore();
}


