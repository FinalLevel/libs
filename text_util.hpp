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
		void base64Encode(fl::strings::BString &result, const char *input, const size_t size, bool isFormated = true);
		
		using TStringSet = std::unordered_set<std::string>;
		void stripHtmlTags(fl::strings::BString &buf, const TStringSet &allowedTags = {});
		void stripHtmlTags(const char *src, const size_t size, fl::strings::BString &buf, const TStringSet &allowedTags = {});
		// decodeHtmlEntities converts HTML entities to UTF8 chars
		void decodeHtmlEntities(fl::strings::BString &result);
		void decodeMimeHeader(const fl::strings::BString &src, fl::strings::BString &result, const std::string &charset, 
			const char *escapeChars);
		bool stripPreviewText(fl::strings::BString &buf);
		void trimLRText(fl::strings::BString &buf);
		uint getline(const char* start, const char* end);
		void stripBlockquote(const char *src, const size_t size);
		void getTagParamPos(const char *s, const char* e, size_t& start, size_t& end);
		void getTagParamValuePos(const char *s, const char* e, size_t& start, size_t& end);
		bool findTagParamValue(fl::strings::BString& data, const std::string& param, const std::string& val, std::vector<std::tuple<const char*, size_t, std::string>>& vPos);
		void replaceTags(fl::strings::BString &src, const std::vector<std::pair<std::string, std::string>> &v);
	};
};

#endif	// __FL_TEXT_UTIL_HPP
