#pragma once
#ifndef __FL_ICONV_HPP
#define	__FL_ICONV_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Iconv library wrapper functions
///////////////////////////////////////////////////////////////////////////////

#include "bstring.hpp"

namespace fl {
	namespace iconv {
		enum class ECharset : uint32_t
		{
			UNKNOWN,
			UTF8,
			WINDOWS1251,
			WINDOWS1252,
			WINDOWS1254,
			MAX_CHARSET
		};
		struct Charset
		{
			const char * const name;
			const size_t avgSize;
		};
		static const Charset CHARSETS[static_cast<uint32_t>(ECharset::MAX_CHARSET)] =
		{
			{ "UNKNOWN", 0 },
			{ "UTF-8", 2 },
			{ "WINDOWS-1251", 1 },
			{ "WINDOWS-1252", 1 },
			{ "WINDOWS-1254", 1 },
		};
		ECharset getCharsetId(const std::string &charset);
		bool convert(const char *input, const size_t size, fl::strings::BString &result, const ECharset from,
			const ECharset to);
		bool convert(const char *input, const size_t size, fl::strings::BString &result, const char *from, const ECharset to);
	};
};

#endif	// __FL_ICONV_HPP
