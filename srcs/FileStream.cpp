#include "FileStream.hpp"
#include "fcntl.h"


/**========================================================================
* @                           Constructors
*========================================================================**/

FileStream::FileStream() : IStream(-1), filename(), content(), linkConn(NULL) {}
FileStream::FileStream( int fd ) : IStream(fd), filename(), content(), linkConn(NULL) {}
FileStream::FileStream( const string& filename ) : IStream(-1), filename(filename), content(), linkConn(NULL)
{
	// open?
}

FileStream::FileStream( const FileStream& src ) : IStream(src.fd), filename(src.filename), content(src.content),linkConn(src.linkConn) {}
FileStream::~FileStream() {}

/**========================================================================
* *                            operators
*========================================================================**/

FileStream&	FileStream::operator=( const FileStream& src )
{
	if (this != &src)
	{
		this->IStream::operator=(src);
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

ssize_t	FileStream::read()
{
	ssize_t	byte = readFrom(this->fd, this->content);

	return byte;
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
				//   1);
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

void FileStream::dummy() {}
