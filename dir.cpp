///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Directory functions wrapper class
///////////////////////////////////////////////////////////////////////////////

#include <cerrno>
#include <unistd.h>
#include <cstring>
#include "dir.hpp"
#include "bstring.hpp"

using namespace fl::fs;
using fl::strings::BString;

Directory::Directory(const char* path)
{
	_dir = opendir(path);
	if (!_dir)
		throw Error("Cannot open dir");
	bzero(&_entry, sizeof(_entry));
}

Directory::~Directory()
{
	closedir(_dir);
}

bool Directory::next()
{
	struct dirent *result = NULL;
	if (readdir_r(_dir, &_entry, &result) == 0) {
		if (result)
			return true;
		else
			return false;
	}
	return false;
}

void Directory::rewind()
{
	rewinddir(_dir);
}

bool Directory::makeDirRecursive(const char *path, int mode)
{
	BString pathBuf;
	pathBuf << path;
	char *pstr = pathBuf.data();
  char *pathDelimter;
  while ((pathDelimter = strchr(pstr, '/')) != NULL) {
		*pathDelimter = 0;
		if (*pstr) {
			if (mkdir(pathBuf.data(),  mode) && errno != EEXIST)
				return false;
		}
		*pathDelimter = '/';
		pstr = pathDelimter + 1;
  }
	if (mkdir(path,  mode) && errno != EEXIST)
		return false;
	return true;
}

bool Directory::rmDirRecursive(const char *path)
{
	try
	{
		Directory dir(path);
		BString cpath;
		while (dir.next()) {
			cpath.sprintfSet("%s/%s", path, dir.name());
			if (dir.isDirectory()) {
				if (dir.name()[0] == '.' && ((dir.name()[1] == '.' &&  dir.name()[2] == 0) || dir.name()[1] == 0)) {
					// skip ".." and "." 
					continue;
				}
				if (!rmDirRecursive(cpath.c_str()))
					return false;
			}
			else
				if (unlink(cpath.c_str()))
					return false;
		}
		if (rmdir(path))
			return false;
	}
	catch (Error &co)
	{
		return false;
	}
	return true;
}
