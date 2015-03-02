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
#include <ctype.h>
#include <math.h>
#include <algorithm>

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
		
		bool isValidEmail(const char* email)
		{
			bool findAt = false;
			bool findDomainPoint = false;
			int domainLength = 0;
			int emailLength = 0;
			
			const char *pStr = email;
			while (*pStr) {
				unsigned char ch = *pStr;
				if (!isascii(ch)) {
					return false;
				}
				if (ch == '@') {
					if (findAt) {
						return false;
					}
					if (!emailLength) {
						return false;
					}
					findAt = true;
					domainLength = 0;
				} else if (ch == '.') {
					if (findAt) {
						if (!domainLength) {
							return false;
						}
						findDomainPoint = true;
						domainLength = 0;
					}
				} else {
					if (findAt) {
						if (!isalpha(ch) && !isdigit(ch) && ch != '-') {
							return false;
						}
						domainLength++;
					} else {
						if (!isalpha(ch) && !isdigit(ch) && ch != '-' && ch != '_') {
							return false;
						}
						emailLength++;
					}
				}
				pStr++;
			}
			return findAt && findDomainPoint && domainLength;
		}
		inline uint64_t countryPrefixToUnit64(const uint32_t countryPrefix, const uint32_t nationalLength)
		{
			static const uint64_t pow10[15] = {
        1, 10, 100, 1000, 10000, 
        100000, 1000000, 10000000, 100000000, 1000000000, 10000000000ULL, 100000000000ULL, 100000000000ULL,
				1000000000000ULL, 10000000000000ULL
			};
			return countryPrefix * pow10[nationalLength];
		}
		
		uint64_t formInternationalPhone(const std::string &phone, const uint32_t countryPrefix)
		{
			static const size_t MIN_INT_PHONE_LENGTH = 1 /* + */ + 1 /* min country code length */ + 7 /* phone number */; 
			static const size_t MAX_INT_PHONE_LENGTH = 1 /* + */ + 3 /* max country code length */ + 3 /* code */ 
				+ 7 /* phone number */;
			std::string normallizedPhone;
			normallizedPhone.reserve(MAX_INT_PHONE_LENGTH + 1);
			const char *pPhone = phone.c_str();
			while (*pPhone) {
				unsigned char ch = *pPhone;
				if (isdigit(ch) || (ch == '+')) {
					normallizedPhone.push_back(ch);
				}
				pPhone++;
			}
			if (normallizedPhone.size() > MAX_INT_PHONE_LENGTH) {
				return 0;
			}
			pPhone = normallizedPhone.c_str();
			
			if (*pPhone == '+') { // international number
				if (normallizedPhone.size() < MIN_INT_PHONE_LENGTH) {
					return 0;
				}
				return strtoull(pPhone + 1, NULL, 10);
			} else if (*pPhone == '0') {
				static const size_t MIN_NATINAL_PHONE_LENGTH = 7;
				if (normallizedPhone.size() < MIN_NATINAL_PHONE_LENGTH) {
					return 0;
				}
				uint64_t nationalPhone = strtoull(pPhone + 1, NULL, 10);
				if (!nationalPhone) {
					return 0;
				}
				return (countryPrefixToUnit64(countryPrefix, normallizedPhone.size() - 1)) + nationalPhone;
			} else {
				return 0;
			}
		}
		const std::string getFileExt(const std::string &fileName)
		{
			const char *endFile = fileName.c_str() + fileName.size();
			const char *ext = endFile;
			while (ext > fileName.c_str()) {
				if (*ext == '.') {
					ext++;
					size_t len = endFile - ext;
					if (len < 1)
						return std::string();
					std::string extStr(ext, len);
					std::transform(extStr.begin(), extStr.end(), extStr.begin(), ::tolower);
					return extStr;
				}
				ext--;
			}
			return std::string();
		}
	};
};
