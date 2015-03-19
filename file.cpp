///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Direct file IO wrapper class
///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <utime.h>
#include "file.hpp"
#include "bstring.hpp"

using namespace fl::fs;
using fl::strings::BString;

File& File::operator=(File &&moveFrom)
{
	std::swap(moveFrom._descr, _descr);
	return *this;
}

bool File::open(const char *name, const int flags, const mode_t mode)
{
	_descr = ::open(name, flags, mode);
	if (_descr > 0)
		return true;
	else
		return false;
}

void File::close()
{
	if (_descr != 0) {
		::close(_descr);
		_descr = 0;
	}
}

ssize_t File::read(void *buf, const size_t size)
{
	return ::read(_descr, buf, size);
}

ssize_t File::write(const void *buf, const size_t size)
{
	return ::write(_descr, buf, size); 
}

ssize_t File::pread(void *buf, const size_t size, const off_t offset)
{
	return ::pread(_descr, buf, size, offset);
}

ssize_t File::pwrite(const void *buf, const size_t size, const off_t offset)
{
	return ::pwrite(_descr, buf, size, offset);
}

bool File::createUnlinkedTmpFile(const char* tmpDir)
{
	BString fileNameTempl;
	fileNameTempl.sprintfSet("%s/~%u.XXXXXX", tmpDir, getpid());
	_descr = mkstemp(fileNameTempl.c_str());
	if (_descr > 0)
		return (unlink(fileNameTempl.c_str()) == 0);
	else
		return false;
}

ssize_t File::fileSize()
{
	ssize_t curPos = lseek(_descr, 0, SEEK_CUR);
	ssize_t size = lseek(_descr, 0, SEEK_END);
	lseek(_descr, curPos, SEEK_SET);
	return size;
}

off_t File::seek(off_t offset, int whence)
{
	return lseek(_descr, offset, whence);
}

#ifndef NO_LSEEK64
int64_t File::seek64(int64_t offset, int whence)
{
	return lseek64(_descr, offset, whence);
}
#endif

bool File::truncate(const off_t size)
{
	return (ftruncate(_descr, size) == 0);
}

bool File::touch(const char *fileName, const time_t modTime)
{
	utimbuf tb;
	tb.modtime = modTime;
	tb.actime = modTime;
	if (utime(fileName, &tb) == 0)
		return true;
	else {
		File fd;
		if (!fd.open(fileName, O_CREAT | O_WRONLY))
			return false;
		fd.close();
		return (utime(fileName, &tb) == 0);
	}
}

bool File::readAll(fl::strings::BString &msg, const char *name)
{
	File fd;
	if (!fd.open(name, O_RDONLY)) {
		return false;
	}
	ssize_t size = fd.fileSize();
	if (fd.read(msg.reserveBuffer(size), size) != size) {
		msg.clear();
		return false;
	}
	return true;
}
