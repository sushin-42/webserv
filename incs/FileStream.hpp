#ifndef FILESTREAM_HPP
# define FILESTREAM_HPP

# include "utils.hpp"
# include "Stream.hpp"
# include "Undone.hpp"

class ConnSocket;

class FileStream : public Stream
{
/**========================================================================
* %                          member variables
*========================================================================**/
private:
	string		filename;
public:
	string		content;
	ConnSocket*	linkConn;


/**========================================================================
* @                           Constructors
*========================================================================**/

public:
	FileStream();
	FileStream( int fd );
	FileStream( const string& filename );
	FileStream( const FileStream& src );
	~FileStream();

/**========================================================================
* *                            operators
*========================================================================**/

	FileStream&	operator=( const FileStream& src );

/**========================================================================
* #                          member functions
*========================================================================**/

	int				open(int mode);
	int				open(const string& filename, int mode);
	ssize_t			read();
	void			send(const string& content, map<int, struct undone>& writeUndoneBuf);
	void			setFilename( const string& filename);
	const string&	getFilename() const;

private:
	void dummy();
};

#endif

