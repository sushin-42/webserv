#ifndef REQBODY_HPP
# define REQBODY_HPP

#include "IText.hpp"
#include "utils.hpp"
#include <exception>
#include <iostream>
# include <string>
# include <sys/fcntl.h>
# include <unistd.h>
# include <fstream>
using namespace std;


class ReqBody : public IText
{
/**========================================================================
* %                          member variables
*========================================================================**/
friend class ConnSocket;
	struct {
		unsigned int				state;
		long long					size;
		long long					total;
		string						data;
		string::size_type			cur;
		string::size_type			newChunkStart;
		string						trailingData;

		bool						isAllDone() { return state ==  8 ;}
	}								chunk;

/**========================================================================
* @                           Constructors
*========================================================================**/
public:
	ReqBody(): IText() {clearChunk();}
	ReqBody( const ReqBody& src ): IText(src.content){}
	~ReqBody() {}

/**========================================================================
* *                            operators
*========================================================================**/

	ReqBody&	operator=( const ReqBody& src );

/**========================================================================
* #                          member functions
*========================================================================**/
	void	clearChunk();

	void	appendChunk(const string& newdata);
	void	setChunk(const string& newdata);

	bool	canGoAhead(const string& s, string::size_type cur, string::size_type i);

	void	printState(int state);
	void	decodingChunk(ssize_t bodyLimit);
	void	clear();

	class invalidChunk: public exception
	{
		private:	string msg;
		public:		explicit invalidChunk(): msg("") {}
					explicit invalidChunk(const string& m): msg(m) {}
					virtual ~invalidChunk() throw() {};
					virtual const char * what() const throw() { return msg.c_str(); }
	};
	class limitExeeded: public exception
	{
		private:	string msg;
		public:		explicit limitExeeded(): msg("") {}
					explicit limitExeeded(const string& m): msg(m) {}
					virtual ~limitExeeded() throw() {};
					virtual const char * what() const throw() { return msg.c_str(); }
	};

};
#endif
