#ifndef PIPE_HPP
# define PIPE_HPP
# include "IStream.hpp"

class Pipe : public IStream
{
public:
	string content;

public:
	Pipe()
	: IStream(), content() {};

	Pipe( int fd )
	:IStream(fd), content() {};

	Pipe( const Pipe& src )
	: IStream(src), content(src.content) {}

	~Pipe() {};

	Pipe&	operator=( const Pipe& src )
	{
		if (this != &src)
		{
			this->IStream::operator=(src);
			this->content = src.content;
		}
		return *this;
	}
private:
	void dummy() {}
};


#endif
