#pragma once
#ifndef __FL_UTIL_HPP
#define	__FL_UTIL_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Different utilities functions
///////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <stdlib.h>

namespace fl {
	namespace utils {
		
		template <class T> T convertStringTo(const char *src, char **nextChar = NULL, int base = 10);

		template <>
		inline int convertStringTo<int>(const char *src, char **nextChar, int base)
		{
			return strtol(src, nextChar, base);
		}
		template <>
		inline uint8_t convertStringTo<uint8_t>(const char *src, char **nextChar, int base)
		{
			return strtoul(src, nextChar, base);
		}
		template <>
		inline uint16_t convertStringTo<uint16_t>(const char *src, char **nextChar, int base)
		{
			return strtoul(src, nextChar, base);
		}

		template <>
		inline uint32_t convertStringTo<uint32_t>(const char *src, char **nextChar, int base)
		{
			return strtoul(src, nextChar, base);
		}
		
		template <>
		inline unsigned long convertStringTo<unsigned long>(const char *src, char **nextChar, int base)
		{
			return strtoull(src, nextChar, base);
		}
		
		template <>
		inline unsigned long long convertStringTo<unsigned long long>(const char *src, char **nextChar, int base)
		{
			return strtoull(src, nextChar, base);
		}
		template <>
		inline float convertStringTo<float>(const char *src, char **nextChar, int base)
		{
			return strtof(src, nextChar);
		}
		template <>
		inline double convertStringTo<double>(const char *src, char **nextChar, int base)
		{
			return strtod(src, nextChar);
		}
		
		template <class T> const T convertStdStringTo(const std::string &src, char **nextChar = NULL, int base = 10)
		{
			return convertStringTo<T>(src.c_str(), nextChar, base);
		}

		template <>
		inline const std::string convertStdStringTo<std::string>(const std::string &src, char **nextChar, int base)
		{
			return src;
		}
		
		inline u_int32_t getCheckSum32(const char *str, u_int32_t len, u_int32_t sum = 0)
		{
			for (u_int32_t i = 0; i < len; i++) {
				sum = str[i] + (sum << 5) + (sum << 12) - sum;
			}

			return sum;
		}
		
		inline u_int32_t getCheckSum64(const char *str, u_int32_t len, u_int32_t sum = 0)
		{
			for (size_t i = 0; i < len; i++) {
				sum = str[i] + (sum << 10) + (sum << 24) - sum;
			}
			return sum;
		}
		
		template<class T> inline u_int32_t getCheckSum32Tmpl(const T& value)
		{
			return value;
		}
		template<>
		inline u_int32_t getCheckSum32Tmpl<std::string>(const std::string& value)
		{
			return getCheckSum32(value.c_str(), value.size());
		}
		
		template <typename T> bool explode(const char *str, std::vector<T> &array, char delimiter = ',', int base = 10)
		{
			if (!str)
				return false;
			
			const char *pStr = str;
			const char *pEnd = pStr + strlen(str);
			while (pStr < pEnd) {
				char *pNext;
				auto val = convertStringTo<T>(pStr, &pNext, base);
				array.push_back(val);
				if(*pNext != delimiter)
					break;
				pStr = pNext + 1;
			}
			return !array.empty();
		}	

		bool fileExists(const char *fileName);
		size_t parseSizeString(const char *str);
		
		inline uint8_t digit2Int(const char d) {
			return (d & 0x1f) + ((d >> 6) * 0x19) - 0x10;
		}
		void hex2BinaryBackOrder(const char *hexStr, uint8_t *binary, const size_t binarySize);
		void hex2Binary(const char *hexStr, uint8_t *binary, const size_t binarySize);
		
		bool enableCoreDump();
		bool setMaxOpenFiles(const int maxOpenFiles);
		bool setMaxProcess(const int maxProcess);
		
		
		bool isValidEmail(const char *email, size_t len = 0);
		
		
		inline char *strncasestr(const char *s, const char *find, size_t slen)
		{
			size_t paternLen = strlen(find);
			if (slen < paternLen)
				return 0;

			const char *end = s + slen - paternLen;
			const char *p = s;
			int res = -1;
			while (*p && p <= end && (res = strncasecmp(p, find, paternLen)))
				p++;

			return (char *) (res ? 0 : p);
		}
		
		template <typename T> T toNumberSignificantFirst(const uint8_t *bytes, const size_t length)
		{
		  T sum = 0;
			for(size_t i = 0; i < length; i++) {
				const size_t shift = (length - 1 - i) * 8;
				sum |= static_cast<T>(bytes[i]) << shift;
			}
			return sum;
		}
		
		const std::string getFileExt(const std::string &name);
		const char *strnstr(const char *s, const size_t slen, const char *pattern, const size_t findLen);
		const char *rstrnstr(const char *s, const size_t slen, const char *pattern, const size_t findLen);
		
		struct Url
		{
			std::string schema;
			std::string host;
			std::string path;
			std::string query;
			uint32_t port;
		};
		bool parseUrl(const std::string &rawUrl, Url &url);

	};
};

#endif	// __FL_UTIL_HPP
