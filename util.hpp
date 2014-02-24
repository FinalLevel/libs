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
#include <string>

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
		
		u_int32_t getCheckSum32(const char *str, u_int32_t len, u_int32_t sum = 0)
		{
			for (u_int32_t i = 0; i < len; i++) {
				sum = str[i] + (sum << 5) + (sum << 12) - sum;
			}

			return sum;
		}
		
		template<class T> u_int32_t getCheckSum32Tmpl(const T& value)
		{
			return value;
		}
		template<>
		u_int32_t getCheckSum32Tmpl<std::string>(const std::string& value)
		{
			return getCheckSum32(value.c_str(), value.size());
		}

	};
};

#endif	// __FL_UTIL_HPP
