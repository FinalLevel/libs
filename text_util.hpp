#pragma once
#ifndef __FL_TEXT_UTIL_HPP
#define	__FL_TEXT_UTIL_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Different text processing utility functions 
///////////////////////////////////////////////////////////////////////////////

#include <unordered_set>
#include <string>
#include "bstring.hpp"

namespace fl {
	namespace utils {
		void quotedPrintableDecode(fl::strings::BString &result, const char *input, const size_t size, 
			const char delim = '=');
		bool base64Decode(fl::strings::BString &result, const char *input, const size_t size);
		
		using TStringSet = std::unordered_set<std::string>;
		void stripHtmlTags(fl::strings::BString &buf, const TStringSet &allowedTags = {});
		void stripHtmlTags(const char *src, const size_t size, fl::strings::BString &buf, const TStringSet &allowedTags = {});
		// decodeHtmlEntities converts HTML entities to UTF8 chars
		void decodeHtmlEntities(fl::strings::BString &result);
		void decodeMimeHeader(const fl::strings::BString &src, fl::strings::BString &result, const std::string &charset, 
			const char *escapeChars);
	};
};

#endif	// __FL_TEXT_UTIL_HPP
