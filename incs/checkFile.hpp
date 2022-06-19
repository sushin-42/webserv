#ifndef CHECKFILE_HPP
# define CHECKFILE_HPP
#include <string>
#include <sys/stat.h>
#include <dirent.h>
#include "Exceptions.hpp"
using namespace std;

void	_checkFile(const string& path, bool auto_index);
string	directoryListing(const string& path, const string& uri);

#endif
