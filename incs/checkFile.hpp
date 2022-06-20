#ifndef CHECKFILE_HPP
# define CHECKFILE_HPP
#include <string>
#include <sys/stat.h>
#include <dirent.h>
#include "httpError.hpp"
#include "Config.hpp"
using namespace std;

struct stat	_checkFile(const string& path);
string		findFirstMatched(const string& filepath, const vector<string>& indices);
string		directoryListing(const string& filename, const string& prefix);
string		findIndexFile(Config* conf, const string& filename);

class indexIsDirectory: public exception
{
	private:	string msg;
	public:		explicit indexIsDirectory();
				explicit indexIsDirectory(const string& m);
				virtual ~indexIsDirectory() throw();
				virtual const char * what() const throw();
};

#endif
