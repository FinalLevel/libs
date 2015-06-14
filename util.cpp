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
		
		bool isValidEmail(const char* email, size_t len)
		{
			bool findAt = false;
			bool findDomainPoint = false;
			int domainLength = 0;
			int emailLength = 0;
			if (!len) {
				len = strlen(email);
			}
			const char *pStr = email;
			const char *end = email + len;
			while (pStr < end) {
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
						if (!isalpha(ch) && !isdigit(ch) && ch != '-' && ch != '+' ) {
							return false;
						}
						domainLength++;
					} else {
						if (!isalpha(ch) && !isdigit(ch) && ch != '-' && ch != '+' && ch != '_') {
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
		
		const char *strnstr(const char *s, const size_t slen, const char *pattern, const size_t patternLen)
		{
			if (slen < patternLen)
				return 0;

			const char *end = s + slen - patternLen;
			const char *p = s;
			int res = -1;
			while (*p && p <= end && (res = memcmp(p, pattern, patternLen)))
				p++;

			return (char *) (res ? 0 : p);		
		}

		const char *rstrnstr(const char *s, const size_t slen, const char *pattern, const size_t patternLen)
		{
			if (slen < patternLen)
				return 0;

			const char *start = s + patternLen;
			const char *p = s + slen - patternLen;
			int res = -1;
			while (*p && p >= start && (res = memcmp(p, pattern, patternLen)))
				p--;

			return (char *) (res ? 0 : p);
		}
		
		bool parseUrl(const std::string &rawUrl, Url &url)
		{
			static const std::string SCHEMA_DELIMITER {"://"};
			const char *pSchema = strstr(rawUrl.c_str(), SCHEMA_DELIMITER.c_str());
			if (!pSchema) {
				return false;
			}
			int len = pSchema - rawUrl.c_str();
			if (len <= 0) {
				return false;
			}
			url.schema.assign(rawUrl.c_str(), len);
			url.port = 0;
			
			const char *pHostStart = pSchema + SCHEMA_DELIMITER.size();
			const char *pHostEnd = nullptr;
			const char *pStr = pHostStart;
			while (*pStr && *pStr != '/') {
				if (*pStr == ':') { // port
					pHostEnd = pStr;
					char *end;
					url.port = strtoul(pHostEnd + 1, &end, 10);
					pStr = end;
					break;
				}
				pStr++;
			}
			if (!pHostEnd) {
				pHostEnd = pStr;
			}
			len = pHostEnd - pHostStart;
			if (len <= 0) {
				return false;
			}
			url.host.assign(pHostStart, len);
			url.path.assign("/", 1);
			url.query.clear();
			if (*pStr) { // url has path
				const char *pathStart = pStr;
				const char *pathEnd = nullptr;
				pStr = strchr(pStr, '?');
				if (pStr) {
					pathEnd = pStr;
					pStr++;
					len = (rawUrl.c_str() + rawUrl.size()) - pStr;
					if (len > 0) {
						url.query.assign(pStr, len);
					}
				} else {
					pathEnd = rawUrl.c_str() + rawUrl.size();
				}
				len = pathEnd - pathStart;
				if (len > 0) {
					url.path.assign(pathStart, len);
				}
			}
			if (url.port == 0) {
				if (url.schema == "http") {
					url.port = 80;
				} else if (url.schema == "https") {
					url.port = 443;
				}
			}
			return true;
		}
	};
};
