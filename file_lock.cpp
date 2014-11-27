///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Basic file locking class implementation
///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include "file_lock.hpp"
#include "file.hpp"
#include "log.hpp"

using namespace fl::fs;
using fl::strings::BString;

FileLock::FileLock(const std::string &fileName, const size_t maxWaitTime)
{
	_lockFileName << fileName << ".lock";
	static const size_t CHECK_MILLISECONDS_TIME = 100; // every 100ms do check
	size_t maxWaitChecks = ((maxWaitTime * 1000) / CHECK_MILLISECONDS_TIME) + 1;
	struct timespec tim;
	tim.tv_sec = 0;
  tim.tv_nsec = CHECK_MILLISECONDS_TIME * 1000000;
	auto currentTime = time(NULL);
	_lockKey = (static_cast<decltype(_lockKey)>(time(NULL)) << 32) | rand();
	for (size_t t = 0; t < maxWaitChecks; t++)
	{
		struct stat fStat;
		if (lstat(_lockFileName.c_str(), &fStat)) { // file doesn't exists write our key and waiting 1 tic
			_writeKey();
		} else if (maxWaitTime && (fStat.st_mtime < (time_t)((currentTime + (t / 1000)) - maxWaitTime))) { // file was lost
			_writeKey();
		} else {
			if (checkKey()) {
				return;
			}
		}
		nanosleep(&tim , NULL);
	}
	throw Error();
}

FileLock::~FileLock()
{
	if (checkKey()) {
		unlink(_lockFileName.c_str());
	}
}

bool FileLock::checkKey()
{
	File fd;
	if (!fd.open(_lockFileName.c_str(), O_RDONLY)) {
		return false;
	}
	decltype(_lockKey) lockKey;
	if (fd.read(&lockKey, sizeof(lockKey)) != sizeof(lockKey)) {
		log::Error::L("Can't read lockKey from %s\n", _lockFileName.c_str());
		return false;
	}
	if (lockKey == _lockKey) {
		return true;
	} else {
		return false;
	}
}

void FileLock::_writeKey()
{
	File fd;
	if (!fd.open(_lockFileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC)) {
		log::Error::L("Can't open lock for writing %s\n", _lockFileName.c_str());
		throw Error();
	}
	if (fd.write(&_lockKey, sizeof(_lockKey)) != sizeof(_lockKey)) {
		log::Error::L("Can't write to lock %s\n", _lockFileName.c_str());
		throw Error();
	}
}
