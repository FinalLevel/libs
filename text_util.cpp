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
#include <unordered_map>
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
			size_t startSize = result.size();
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
			result.trim(startSize + (res - startRes));
		}
		
		static const char BASE64_REVERSE_TABLE[256] = {
			-2, -2, -2, -2, -2, -2, -2, -2, -2, -1, -1, -2, -2, -1, -2, -2,
			-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
			-1, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 62, -2, -2, -2, 63,
			52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -2, -2, -2, -2, -2, -2,
			-2,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
			15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -2, -2, -2, -2, -2,
			-2, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
			41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -2, -2, -2, -2, -2,
			-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
			-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
			-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
			-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
			-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
			-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
			-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
			-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2
		};
		
		bool base64Decode(fl::strings::BString &result, const char *input, const size_t size)
		{
			const char *pCur = input;
			const char *pEnd = pCur + size;
			size_t startSize = result.size();
			unsigned char *startRes = (unsigned char *) result.reserveBuffer(size);
			unsigned char *pRes = startRes;
			int i {0};
			unsigned char ch {0};
			while (pCur < pEnd) {
				ch = *pCur;
				pCur++;
				if (ch == '=') 
					break;
				
				int reverse = BASE64_REVERSE_TABLE[ch];
				if (reverse < 0) {
					continue;
				}
				switch(i % 4) {
					case 0:
						*pRes = reverse << 2;
						break;
					case 1:
						*pRes |= reverse >> 4;
						pRes++;
						*pRes = (reverse & 0x0f) << 4;
						break;
					case 2:
						*pRes |= reverse >>2;
						pRes++;
						*pRes = (reverse & 0x03) << 6;
						break;
					case 3:
						*pRes |= reverse;
						pRes++;
						break;
				}
				i++;
			}
			
			/* mop things up if we ended on a boundary */
			if (ch == '=') {
				switch(i % 4) {
				case 1:
					result.trim(startSize);
					return false;
				case 2:
					pRes++;
				case 3:
					*pRes = 0;
				}
			}
			result.trim(startSize + (pRes - startRes));
			return true;
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
		
		inline char *putUTF8CharByCode(char *res, int num)
		{
			if (num < 128) {
				*res = num;
				res++;
			} else if (num < 2048) {
				*res = (num >> 6) + 192;
				res++;
				*res = (num & 63) + 128;
				res++;
			} else if (num < 65536) {
				*res = (num >> 12) + 224;
				res++;
				*res = ((num >> 6) & 63) + 128;
				res++;
				*res = (num & 63) + 128;
				res++;
			} else if (num < 2097152) {
				*res = (num >> 18) + 240;
				res++;
				*res = ((num >> 12) & 63) + 128;
				res++;
				*res = ((num >> 6) & 63) + 128;
				res++;
				*res = (num & 63) + 128;
				res++;
			}
			return res;
		}
		
		static const size_t MAX_ENTITY_LENGTH = 9;
		using THtmlEntitiesUTF8Map = std::unordered_map<std::string, int>;
		static const THtmlEntitiesUTF8Map HTML_ENTITIES_UTF8_MAP = {
			{ "nbsp", 160 },
			{ "iexcl", 161 },
			{ "cent", 162 },
			{ "pound", 163 },
			{ "curren", 164 },
			{ "yen", 165 },
			{ "brvbar", 166 },
			{ "sect", 167 },
			{ "uml", 168 },
			{ "copy", 169 },
			{ "ordf", 170 },
			{ "laquo", 171 },
			{ "not", 172 },
			{ "shy", 173 },
			{ "reg", 174 },
			{ "macr", 175 },
			{ "deg", 176 },
			{ "plusmn", 177 },
			{ "sup2", 178 },
			{ "sup3", 179 },
			{ "acute", 180 },
			{ "micro", 181 },
			{ "para", 182 },
			{ "middot", 183 },
			{ "cedil", 184 },
			{ "sup1", 185 },
			{ "ordm", 186 },
			{ "raquo", 187 },
			{ "frac14", 188 },
			{ "frac12", 189 },
			{ "frac34", 190 },
			{ "iquest", 191 },
			{ "agrave", 192 },
			{ "aacute", 193 },
			{ "acirc", 194 },
			{ "atilde", 195 },
			{ "auml", 196 },
			{ "aring", 197 },
			{ "aelig", 198 },
			{ "ccedil", 199 },
			{ "egrave", 200 },
			{ "eacute", 201 },
			{ "ecirc", 202 },
			{ "euml", 203 },
			{ "igrave", 204 },
			{ "iacute", 205 },
			{ "icirc", 206 },
			{ "iuml", 207 },
			{ "eth", 208 },
			{ "ntilde", 209 },
			{ "ograve", 210 },
			{ "oacute", 211 },
			{ "ocirc", 212 },
			{ "otilde", 213 },
			{ "ouml", 214 },
			{ "times", 215 },
			{ "oslash", 216 },
			{ "ugrave", 217 },
			{ "uacute", 218 },
			{ "ucirc", 219 },
			{ "uuml", 220 },
			{ "yacute", 221 },
			{ "thorn", 222 },
			{ "szlig", 223 },
			{ "agrave", 224 },
			{ "aacute", 225 },
			{ "acirc", 226 },
			{ "atilde", 227 },
			{ "auml", 228 },
			{ "aring", 229 },
			{ "aelig", 230 },
			{ "ccedil", 231 },
			{ "egrave", 232 },
			{ "eacute", 233 },
			{ "ecirc", 234 },
			{ "euml", 235 },
			{ "igrave", 236 },
			{ "iacute", 237 },
			{ "icirc", 238 },
			{ "iuml", 239 },
			{ "eth", 240 },
			{ "ntilde", 241 },
			{ "ograve", 242 },
			{ "oacute", 243 },
			{ "ocirc", 244 },
			{ "otilde", 245 },
			{ "ouml", 246 },
			{ "divide", 247 },
			{ "oslash", 248 },
			{ "ugrave", 249 },
			{ "uacute", 250 },
			{ "ucirc", 251 },
			{ "uuml", 252 },
			{ "yacute", 253 },
			{ "thorn", 254 },
			{ "yuml", 255 },
			{ "quot", 34 },
			{ "lt", 60 },
			{ "gt", 62 },
			{ "amp", 38 },
		};

		void decodeHtmlEntities(fl::strings::BString &result)
		{
			const char *pBuf = result.c_str();
			const char *end = result.c_str() + result.size();
			char *outBuf = result.data();
			while (pBuf < end) {
				if (*pBuf == '&') {
					const char *pEntity = pBuf + 1;
					if (*pEntity == '#') {
						pEntity++;
						int base = 10;
						if (*pEntity == 'x' || *pEntity == 'X') {
							pEntity++;
							base = 16;
						}
						char *pEnd = 0;
						unsigned int num = strtoul(pEntity, &pEnd, base);
						if (*pEnd != ';' || !num || num >= 2097152) { // skip this entity
							pEntity = pEnd;
						} else {
							outBuf = putUTF8CharByCode(outBuf, num);
							pBuf = pEnd + 1;
							continue;
						}
					} else { // not #
						size_t len = end - pEntity;
						if (len > MAX_ENTITY_LENGTH) {
							len = MAX_ENTITY_LENGTH;
						}
						const char *pEntityEnd = static_cast<const char*>(memchr(pEntity, ';', len));
						if (pEntityEnd) {
							std::string entity(pEntity, pEntityEnd - pEntity);
							std::transform(entity.begin(), entity.end(), entity.begin(), ::tolower);
							auto f = HTML_ENTITIES_UTF8_MAP.find(entity);
							if (f != HTML_ENTITIES_UTF8_MAP.end()) {
								outBuf = putUTF8CharByCode(outBuf, f->second);
								pBuf = pEntityEnd + 1;
								continue;
							}
						} 
					}
					int len = pEntity - pBuf;
					memcpy(outBuf, pBuf, len);
					outBuf += len;
					pBuf = pEntity;
				} else {
					*outBuf = *pBuf;
					outBuf++;
					pBuf++;
				}
			}
			result.trim(outBuf - result.c_str());
		}
	};
};
