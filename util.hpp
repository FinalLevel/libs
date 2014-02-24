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

namespace fl {
	namespace utils {
		
		template <class T> T convertStringTo(const char *src, char **nextChar = NULL, int base = 10) 
		{ 
		}
		
		template <>
		inline int convertStringTo<int>(const char *src, char **nextChar, int base)
		{
			return strtol(src, nextChar, base);
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
	};
};

#endif	// __FL_UTIL_HPP
