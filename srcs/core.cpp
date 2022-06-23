# include "core.hpp"
#include "Exceptions.hpp"
#include "FileStream.hpp"
#include "Pipe.hpp"
# include "checkFile.hpp"
# include "ConfigLoader.hpp"
# include "ConfigChecker.hpp"
#include "httpError.hpp"
#include <string>
# include <sys/stat.h>

status_code_t	deleteFile(pair<string, string> p)
{
	int		ret;
	struct 	stat s;
	string	filename = p.first + p.second;

	try						{ s = _checkFile(filename); }
	catch (httpError& e)	{ throw; }	// for 403

	if (S_ISDIR(s.st_mode) && filename.back() != '/')
		return 409; // throw?

	ret = remove(filename.c_str());
	if (ret == -1)
		return 404; // throw?
	return 204; // throw?
}
