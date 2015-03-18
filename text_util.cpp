///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Different text processing utility functions implementation
///////////////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include <algorithm>
#include "util.hpp"
#include "text_util.hpp"

namespace fl {
	namespace utils {
		inline char hex2int(unsigned char c)
		{
			if ( isdigit(c) )
			{
				return c - '0';
			}
			else if ( c >= 'A' && c <= 'F' )
			{
				return c - 'A' + 10;
			}
			else if ( c >= 'a' && c <= 'f' )
			{
				return c - 'a' + 10;
			}
			else
			{
				return -1;
			}
		}
		
		void quotedPrintableDecode(fl::strings::BString &result, const char *input, const size_t size, 
			const char delim)
		{
			const char *end = input + size;
			char *res = result.reserveBuffer(size);
			const char *startRes = res;
			while (input < end) {
				if (*input == delim) {
					const char *next1 = input + 1;
					const char *next2 = input + 2;
					if (next2 < end && isxdigit(*next1) && isxdigit(*next2)) {
						unsigned char ch = (hex2int(*next1) << 4) + hex2int(*next2);
						*res = ch;
						res++;
						input += 3;
					} else {  /* check for soft line break according to RFC 2045*/
						const char *lb = input + 1;
						while (lb < end && (*lb == ' ' || *lb == '\t')) {
							 /* Possibly, skip spaces/tabs at the end of line */
							lb++;
						}
						if (lb >= end) {
							/* End of line reached */
							input = end;
						} else if (*lb == '\r' && (lb + 1 < end) && *(lb + 1) == '\n') {
							/* CRLF */
							input = lb + 2;
						} else if (*lb == '\r' || *lb == '\n') {
							/* CR or LF */
							input = lb + 1;
						} else {
							*res = *input;
							res++;
							input++;
						}
					}
				} else {
					*res = *input;
					res++;
					input++;
				}
			}
			result.trim(res - startRes);
		}
		
		static const std::unordered_set<std::string> SKIPED_CONTAINERS = {
			"head", "script", "style", "object", "applet", "iframe"
		};
		
		inline char *addSpaceCompression(const unsigned char ch, char *outBuf, const char *startOutBuf)
		{
			if (isspace(ch) && ((startOutBuf == outBuf) || isspace(*(outBuf-1)))) {
				return outBuf;
			}
			*outBuf = ch;
			return (outBuf + 1);
		}
		
		inline char *addSpaceCompressing(const char *src, const char *end, char *outBuf, const char *startOutBuf)
		{
			while (src < end) {
				outBuf = addSpaceCompression(*src, outBuf, startOutBuf);
				src++;
			}
			return outBuf;
		}
		
		size_t stripHtmlTags(const char *src, const size_t size, char *outBuf, const TStringSet &allowedTags)
		{
			char *pOutBuf = outBuf;
			const char *pSrc = src;
			const char *end = src + size;
			std::string tagName;
			while (pSrc < end) {
				if (*pSrc == '<') {
					if (pOutBuf != pSrc) { // check for the same buffer stripping
						pOutBuf = addSpaceCompression(' ', pOutBuf, outBuf);
					}
					pSrc++;
					if (!*pSrc) {
						break;
					}
					static const std::string HTML_COMMENT_START { "!--" };
					if (!strncmp(pSrc, HTML_COMMENT_START.c_str(), HTML_COMMENT_START.size())) {
						pSrc += HTML_COMMENT_START.size();
						static const std::string HTML_COMMENT_END { "-->" };
						pSrc = fl::utils::strnstr(pSrc, end - pSrc, HTML_COMMENT_END.c_str(), HTML_COMMENT_END.size());
						if (pSrc) {
							pSrc += HTML_COMMENT_END.size();
							continue;
						} else {
							break;
						}
					}
					const char *endTagName = pSrc;
					while ((endTagName < end) && (*endTagName) != '>' && !isspace(*endTagName)) {
						endTagName++;
					}
					if (endTagName >= end) {
						break;
					}
					size_t tagNameSize = endTagName - pSrc;
					if (tagNameSize) {
						const char *pTagEnd = static_cast<const char*>(memchr(pSrc + tagNameSize, '>', end - pSrc - tagNameSize));
						if (!pTagEnd) {
							break;
						}
						if (*pSrc == '/') {
							tagName.assign(pSrc + 1, tagNameSize - 1);
						} else {
							tagName.assign(pSrc, tagNameSize);
						}
						std::transform(tagName.begin(), tagName.end(), tagName.begin(), ::tolower);
						if (allowedTags.find(tagName) != allowedTags.end()) {
							pTagEnd++;
							pOutBuf = addSpaceCompressing(pSrc - 1, pTagEnd, pOutBuf, outBuf);
							pSrc = pTagEnd;
						} else if ((*pSrc != '/') && (SKIPED_CONTAINERS.find(tagName) != SKIPED_CONTAINERS.end())) {

							const char *pFinishTag = endTagName;
							while ((pFinishTag = static_cast<const char *>(memchr(pFinishTag, '<', end - pFinishTag))) != nullptr) {
								pFinishTag++;
								if (*pFinishTag != '/') {
									continue;
								}
								pFinishTag++;
								if (!strncasecmp(pFinishTag, tagName.c_str(), tagName.size())) {
									pFinishTag = static_cast<const char *>(memchr(pFinishTag, '>', end - pFinishTag));
									break;
								}
							}
							if (pFinishTag) {
								pSrc = pFinishTag + 1;
							} else {
								break;
							}
						} else {
							pSrc = pTagEnd + 1;
						}
					}
				} else {
					pOutBuf = addSpaceCompression(*pSrc, pOutBuf, outBuf);
					pSrc++;
				}
			}
			while (pOutBuf > outBuf && (isspace(*(pOutBuf - 1)))) {
				pOutBuf--;
			}
			return pOutBuf - outBuf;
		}
		
		void stripHtmlTags(fl::strings::BString &buf, const TStringSet &allowedTags)
		{
			char *outBuf = buf.data();
			auto res = stripHtmlTags(buf.c_str(), buf.size(), outBuf, allowedTags);
			buf.trim(res);
		}
		void stripHtmlTags(const char *src, const size_t size, fl::strings::BString &buf, const TStringSet &allowedTags)
		{
			auto res = stripHtmlTags(src, size, buf.reserveBuffer(size + 1), allowedTags);
			buf.trim(res);
		}
	};
};
