#include "checkFile.hpp"
#include "Exceptions.hpp"
#include "utils.hpp"
#include "ConfigChecker.hpp"
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
/*
	struct stat {
		dev_t     st_dev;         // 파일을 담은 장치의 ID
		ino_t     st_ino;         // 아이노드 번호
		mode_t    st_mode;        // 파일 종류 및 모드
		nlink_t   st_nlink;       // 하드 링크 수
		uid_t     st_uid;         // 소유자의 사용자 ID
		gid_t     st_gid;         // 소유자의 그룹 ID
		dev_t     st_rdev;        // 장치 ID (특수 파일인 경우)
		off_t     st_size;        // 총 크기, 바이트 단위
		blksize_t st_blksize;     // 파일 시스템 I/O의 블록 크기
		blkcnt_t  st_blocks;      // 할당된 512B 블록 수

		struct timespec st_atim;  // 최근 접근 시간
		struct timespec st_mtim;  // 최근 수정 시간
		struct timespec st_ctim;  // 최근 상태 변경 시간
	}

 */


// void	_checkFile(const string& path)
struct stat	_checkFile(const string& path)
{
	struct	stat s;
	int		status = 0;

	bzero(&s, sizeof(struct stat));

	status = stat(path.c_str(), &s);
	if (status == -1)
	{
		switch (errno)
		{
		case ENOENT:
		case ENOTDIR:
		case ENAMETOOLONG:
			throw notFound();

		case EACCES:
		case ELOOP:
            throw forbidden();

		default:
			throw internalServerError();
        }
	}
	else if (access(path.c_str(), R_OK) == -1)
		throw forbidden();

	return s;
}

string	findFirstMatched(const string& dirname, const vector<string>& indices)
{
	struct stat ss;
	string		index;

	vector<string>::const_iterator it, ite;
	it = indices.begin(), ite = indices.end();
	for (; it < ite ; it++)
	{
		index = (*it);
		try					{ ss = _checkFile(dirname + index); }
		catch (notFound& e)	{ continue; }

		return index;
	}
	return "";

}

string findIndexFile(Config* conf, const string& filename)
{
	struct stat s;

	string	indexfile;
	string	_filename = filename;
;
	try						{ s = _checkFile(_filename); }
	catch (HTTP_Error& e)	{ throw; }

	if (S_ISDIR(s.st_mode))
	{
		if (_filename[_filename.length() - 1] != '/') { _filename += '/'; }

		indexfile = findFirstMatched(_filename, conf->index);
		if (!indexfile.empty())	// found
			_filename = findIndexFile(conf, _filename + indexfile);
	}
	return _filename;	// DIR or FILE?

	/**========================================================================
	 * @  if FOUND final (deepest) index file
	 * @  	if final index file was directory
	 * @		auto index on ? directory listing() : forbidden();
	 * @	else
	 * @		if final index file forbidden? forbidden();
	 * @		else print index file;
	 * '  else NOT FOUND
	 * '	auto index on ? directory listing() : forbidden();
	 *========================================================================**/
}

string	checkIndex(Config* conf, const string& filename)//, struct stat s )
{
	string			indexfile;

	// if (S_ISDIR(s.st_mode))	//*! index MAYBE work with non-dir file *//
	// {
		try							{ indexfile = findIndexFile(conf, filename); }
		catch (HTTP_Error& e)		{ throw; }

		if (indexfile[indexfile.length() - 1] == '/')
		{
			if (conf->auto_index)
				throw autoIndex(indexfile);
			else
				throw forbidden();
		}
	// }
	return indexfile;
}




	// if (S_ISDIR(s.st_mode))
	// {
	// 	try							{ indexfile = findIndexFile(connected->conf, filename); }
	// 	catch (HTTP_Error& e)		{ throw; }

	// 	if (indexfile.back() == '/')
	// 	{
	// 		if (connected->conf->auto_index)
	// 		{
	// 			connected->ResH.setStatusCode(200);
	// 			connected->ResH.setDefaultHeaders();
	// 			connected->ResB.setContent(directoryListing(indexfile, prefix));	/* alias case: need to append Loc URI || or req Target ? */
	// 			connected->ResH["Content-Length"]	= toString(connected->ResB.getContent().length());
	// 			connected->ResH["Content-Type"]		= "text/html";
	// 			return indexfile;
	// 		}
	// 		else
	// 			throw forbidden();
	// 	}
	// 	filename = indexfile;
	// }
	// return filename;






static void	indexing(vector<string>& dirs, const string& path)
{
	DIR				*dirp;
	struct dirent	*dirent;
	string			filename;
	struct stat     sb;

	dirp = opendir(path.c_str());
	if (dirp)
	{
		dirent = readdir(dirp);
		while (dirent)
		{
			filename.clear();
        	filename = dirent->d_name;
        	if (filename == ".")
			{
				dirent = readdir(dirp);
            	continue;
			}

        	bzero(&sb, sizeof(sb));
        	if (stat((path + filename).c_str(), &sb) != 0)
			{
				cout << "error whild indexing : " << path + filename <<endl;
            	throw exception();
			}
        	if (S_ISDIR(sb.st_mode))
            	filename += "/";

			dirs.push_back(filename);
			dirent = readdir(dirp);
		}
		closedir(dirp);
		sort(dirs.begin(), dirs.end());
	}
}

string    directoryListing(const string& filename, const string& prefix)
{
	string body;
	vector<string> dirs;
	vector<string>::iterator it, ite;

	indexing(dirs, filename);
	it = dirs.begin(), ite = dirs.end();

	/* remove prefix (system filepath) */
	string uri = filename.substr(prefix.length());
    body = "<html>\r\n<head><title>Index of " + uri + "</title></head>\r\n<body>\r\n<h1>Index of " + uri +"</h1><hr><pre>";

	for (; it < ite ; it++)
	{
        body += "<a href=\"";
        body += *it;
        body += "\">";

		body += (it->back() == '/') ? "&#x1F4C1 " : "&#x1F4C4 ";
		body += *it;
        body += "</a>\r\n";
    }

	body += "</pre><hr></body>\r\n</html>";

    return body;
}


indexIsDirectory::indexIsDirectory(): msg(""){}
indexIsDirectory::indexIsDirectory(const string& m): msg(m) {}
indexIsDirectory::~indexIsDirectory() throw() {};
const char*	indexIsDirectory::what() const throw() { return msg.c_str(); }
