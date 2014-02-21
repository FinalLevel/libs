///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Direct file IO wrapper class
///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include "file.hpp"
#include "bstring.hpp"

using namespace fl::fs;
using fl::strings::BString;

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
