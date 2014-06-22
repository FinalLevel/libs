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

namespace fl {
	namespace utils {
		
		template <class T> T convertStringTo(const char *src, char **nextChar = NULL, int base = 10);

		template <>
		inline int convertStringTo<int>(const char *src, char **nextChar, int base)
		{
			return strtol(src, nextChar, base);
		}
		template <>
		inline u_int8_t convertStringTo<u_int8_t>(const char *src, char **nextChar, int base)
		{
			return strtoul(src, nextChar, base);
		}
		template <>
		inline u_int16_t convertStringTo<u_int16_t>(const char *src, char **nextChar, int base)
		{
			return strtoul(src, nextChar, base);
		}

		template <>
		inline u_int32_t convertStringTo<u_int32_t>(const char *src, char **nextChar, int base)
		{
			return strtoul(src, nextChar, base);
		}
		template <>
		inline u_int64_t convertStringTo<u_int64_t>(const char *src, char **nextChar, int base)
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
			for (register size_t i = 0; i < len; i++) {
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
	};
};

#endif	// __FL_UTIL_HPP
