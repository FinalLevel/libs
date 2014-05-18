///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Different utilities functions
///////////////////////////////////////////////////////////////////////////////

#include <sys/stat.h>
#include "util.hpp"

namespace fl {
	namespace utils {

		bool fileExists(const char *fileName)
		{
			struct stat fStat;
			if (lstat(fileName, &fStat))
				return false;
			else
				return true;
		}
		
		size_t parseSizeString(const char *str)
		{
			char *last;
			auto size = convertStringTo<size_t>(str, &last, 10);
			if (size == 0)
				return 0;
			if (tolower(*last) == 'k')
				return (size * 1024);
			if (tolower(*last) == 'm')
				return (size * 1024 * 1024);
			if (tolower(*last) == 'g')
				return (size * 1024 * 1024 * 1024);
			return size;
		}
	};
};
