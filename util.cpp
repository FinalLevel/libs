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
#ifndef NO_SYS_PRCTL
#include <sys/prctl.h>
#endif

#include <sys/resource.h>
#include <unistd.h>

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
		
		void hex2Binary(const char *hexStr, uint8_t *binary, const size_t binarySize)
		{
			uint8_t lower, upper; 
			
			for (size_t i = 0; i < binarySize; i++) {
			 upper = hexStr[2 * i + 0]; 
			 lower = hexStr[2 * i + 1];
			 binary[i] = (digit2Int(upper) << 4) | digit2Int(lower);
			}
		}
		void hex2BinaryBackOrder(const char *hexStr, uint8_t *binary, const size_t binarySize)
		{
			uint8_t lower, upper; 
			
			uint8_t *pBin = binary + binarySize - sizeof(uint8_t);
			for (size_t i = 0; i < binarySize; i++) {
			 upper = hexStr[2 * i + 0]; 
			 lower = hexStr[2 * i + 1];
			 *pBin = (digit2Int(upper) << 4) | digit2Int(lower);
			 pBin--;
			}
		}
		
#ifndef NO_SYS_PRCTL
		bool enableCoreDump()
		{
			prctl(PR_SET_DUMPABLE, 1);
			struct rlimit limit;
			limit.rlim_cur = limit.rlim_max = RLIM_INFINITY;
			if (setrlimit(RLIMIT_CORE, &limit) == 0) {
				return true;
			} else {
				return false;
			}
		}
#endif
		
		bool setMaxOpenFiles(const int maxOpenFiles)
		{
			struct rlimit limit;
			limit.rlim_cur = limit.rlim_max = maxOpenFiles;
			if (setrlimit(RLIMIT_NOFILE, &limit) == 0) {
				return true;
			} else {
				return false;
			}
		}
		
		bool setMaxProcess(const int maxProcess)
		{
			struct rlimit limit;
			limit.rlim_cur = limit.rlim_max = maxProcess;
			if (setrlimit(RLIMIT_NPROC, &limit) == 0) {
				return true;
			} else {
				return false;
			}
		}
	};
};
