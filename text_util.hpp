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
#include <unordered_map>
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
		void stripPreviewText(fl::strings::BString &buf);
		void trimLRText(fl::strings::BString &buf);
		uint getline(const char* start, const char* end);
		void stripBlockquote(const char *src, const size_t size);
		void replaceTags(fl::strings::BString &src, const std::vector<std::pair<std::string, std::string>> &v);
	};
};

#endif	// __FL_TEXT_UTIL_HPP
