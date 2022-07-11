#ifndef FILESTREAM_HPP
# define FILESTREAM_HPP

# include "utils.hpp"
# include "Stream.hpp"


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
	int				open(int mode, int permission);
	int				open(const string& filename, int mode);
	void			setFilename( const string& filename);
	const string&	getFilename() const;
	// void			readInputFileStream();

	void			recv();
	void			coreDone();
	void			send(const string& content);
	void			core();
	string			getOutputContent();

private:
	void dummy();
};

/**========================================================================
* ,                               Others
*========================================================================**/

void	createInputFileStream(ConnSocket* connected, const string& filename);

#endif

