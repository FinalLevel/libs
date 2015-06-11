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
#include <map>
#include "util.hpp"
#include "text_util.hpp"
#include "iconv.hpp"

namespace fl {
	namespace utils {
		using fl::strings::BString;
		
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

		static const char BASE64_TABLE[] =
		    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

		void base64Encode(fl::strings::BString &result, const char *input, const size_t size, bool isFormated)
		{
			size_t i;
			char *pos;
			size_t formated {0};
			size_t lineLength {0};

			if (isFormated)
				lineLength = 72;

		    result.reserveBuffer(size*2);
		    pos = result.data();

		    for (i = 0; i < size - 2; i += 3)
		    {
		    	*pos++ = BASE64_TABLE[(input[i] >> 2) & 0x3F];
		    	*pos++ = BASE64_TABLE[((input[i] & 0x3) << 4) | ((input[i + 1] & 0xF0) >> 4)];
		    	*pos++ = BASE64_TABLE[((input[i + 1] & 0xF) << 2) | ((input[i + 2] & 0xC0) >> 6)];
		    	*pos++ = BASE64_TABLE[input[i + 2] & 0x3F];
		    	formated += 4;
		    	if (lineLength > 0 && formated >= lineLength)
		    	{
		    		*pos++ = '\r';
		    		*pos++ = '\n';
		    		formated = 0;
		    	}
		    }
		    if (i < size) {
		    	*pos++ = BASE64_TABLE[(input[i] >> 2) & 0x3F];
		    	if (i == (size - 1)) {
		    		*pos++ = BASE64_TABLE[((input[i] & 0x3) << 4)];
		    		*pos++ = '=';
		    	}
		    	else {
		    		*pos++ = BASE64_TABLE[((input[i] & 0x3) << 4) | ((input[i + 1] & 0xF0) >> 4)];
		    		*pos++ = BASE64_TABLE[((input[i + 1] & 0xF) << 2)];
		    	}
		    	*pos++ = '=';
		    }

		    result.trim(pos - result.data());
		}
		
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
						*pRes |= reverse >> 2;
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
					if (*pRes) {	
						pRes++;
					}
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
		
		const char allowedMimeChars[256] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};
		
		bool checkXlatChars(const char *str, size_t len)
		{
			for (size_t i = 0; i < 256 && i < len; i++, str++) {
				if (!allowedMimeChars[(unsigned char) (*str)]) {
					//printf("%20s\nwrong char '%c'(%d)\n", str, *str, (unsigned char) *str);
					return false;
				}
			}

			return true;
		}
		
		void decodeMimeHeader(const fl::strings::BString &src, fl::strings::BString &result, const std::string &globalCharset, 
			const char *escapeChars)
		{
			result.clear();
			if (!globalCharset.empty() && !checkXlatChars(src.c_str(), src.size())) {
				fl::iconv::convert(src.c_str(), src.size(), result, globalCharset.c_str(), fl::iconv::ECharset::UTF8);
				return;
			}
			BString buffer;
			const char *pCur = src.c_str();
			const char *end = src.c_str() + src.size();
			while (pCur < end) {
				auto len = end - pCur;
				static const std::string START_MARKER{ "=?"};
				const char *p = strnstr(pCur, len, START_MARKER.c_str(), START_MARKER.size());
				if (!p) {
					result.add(pCur, len);
					break;
				}
				len = p - pCur;
				if (len) {
					for (const char *text = pCur; text < p; text++) {
						const char ch = *text;
						if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n' && ch != '\0' && ch != '\x0B') {
							result.add(pCur, len);
							break;
						}
					}
					pCur += len;
				}
				
				const char *character = pCur + START_MARKER.size(); // "=?"
				p = (const char *) memchr(character, '?', end - character);
				if (!p) {
					len = end - pCur;
					result.add(pCur, len);
					break;
				}
				const char *pEncoding = p + 1;
				const char encoding = toupper(*pEncoding); // "?"
				const char *text = p + 2; // "b?" || "q?"
				if ((encoding != 'B' && encoding != 'Q') || *text != '?') {
					result.add(pCur, 2);
					pCur += 2;
					continue;
				}
				text++;
				static const std::string END_MARKER{ "?="};
				p = strnstr(text, end - text, END_MARKER.c_str(), END_MARKER.size());
				if (!p) {
					len = end - pCur;
					result.add(pCur, len);
					break;
				}
				len = p - text;
				if (len <= 0) { // skip convert if size = 0
					pCur = p + 2; // "?="
					continue;
				}
				std::string charset(character, pEncoding - 1 - character);
				auto charsetId = fl::iconv::getCharsetId(charset);
				
				auto currentResult = result.size();
				BString *res = &result;
				if (charsetId != fl::iconv::ECharset::UTF8) {
					buffer.clear();
					res = &buffer;
				}
				if (encoding == 'B') {
					base64Decode(*res, text, len);
				} else {
					quotedPrintableDecode(*res, text, len);
				}
				if (charsetId != fl::iconv::ECharset::UTF8) {
					fl::iconv::convert(buffer.c_str(), buffer.size(), result, charset.c_str(), fl::iconv::ECharset::UTF8);
				}
				if (escapeChars) {
					buffer.clear();
					const char *esc = result.c_str() + currentResult;
					const char *cur = esc;
					while ((cur = strpbrk(esc, escapeChars)) != nullptr) {
						buffer.add(esc, cur - esc);
						buffer << '\\';
						buffer << *cur;
						esc = cur + 1;
					}
					buffer.add(esc, result.c_str() + result.size() - esc);
					if (!buffer.empty()) {
						result.trim(currentResult);
						result << buffer;
					}
				}
				pCur = p + 2;
			}
		}

		void stripBlockquote(const char *src, const size_t size)
		{
			const char* bqStart;
			const char* bqEnd;
			static const char* bqStartTag {"<blockquote"};
			static const char* bqEndTag {"</blockquote>"};
			static size_t bqOpenSize = strlen(bqStartTag);
			static size_t bqCloseSize = strlen(bqEndTag);

			bqStart = fl::utils::strnstr(src, size, bqStartTag, bqOpenSize);
			if (bqStart)
			{
				bqEnd = fl::utils::rstrnstr(bqStart + bqOpenSize, size - (bqStart + bqOpenSize - src), bqEndTag, bqCloseSize);
				if (bqEnd)
				{
					char *dst = const_cast<char *>(bqStart);
					*(dst) = '\n';
					*(dst+1) = '>';
					*(dst+2) = '\n';
					memcpy((dst+3), bqEnd + bqCloseSize, size - (bqEnd + bqCloseSize - src) + 1);
				}
			}
		}

		void replaceTags(fl::strings::BString &buf, const std::vector<std::pair<std::string, std::string>> &v)
		{
			char *start = buf.c_str();
			char *end = buf.c_str() + buf.size();
			char *cur = start;
			char *pos = start;
			char* bEnd = {nullptr};
			size_t size = buf.size();

			BString res;
			res.reserve(size);

			while (pos)
			{
				for (auto &entry : v)
				{
					if((pos = const_cast<char *>(strnstr(pos, size, entry.first.c_str(), entry.first.size()))))
					{
						if ( ((pos - start >=1) && *(pos-1) == '<') /*|| ((pos - start >=2) && *(pos-2) == '<' && *(pos-1) == '/')*/)
						{
							bEnd = pos + entry.first.size();
							while(bEnd < end && *bEnd != '>')
								bEnd++;
							if (bEnd == end)
								return;

							*(pos-1) = 0;
							res << cur << entry.second.c_str();
							pos = bEnd + 1;
							size = end - pos;
							cur = pos;
						}
						else
							pos += entry.first.size();
					}
				}
			}
			if(cur != buf.c_str())
			{
				res << cur;
				buf = std::move(res);
			}
		}

		uint getline(const char* buf, const char* end)
		{
			const char *start = buf;
			while ( buf < end) {
				if (*buf == '\n') {
					return buf - start + 1;
				}
				buf++;
			}
			return buf - start;
		}

		void trimLRText(BString &buf)
		{
			buf.trimLastSpaces();

			const char *pBuf = buf.c_str();
			const char *pEnd = buf.c_str() + buf.size();

			while(pEnd > pBuf)
			{
				if(!isspace(*pBuf))
					break;
				pBuf++;
			}
			if (pBuf > buf.c_str())
			{
				size_t size = strlen(pBuf);
				memcpy(buf.c_str(), pBuf, size);
				buf.trim(size);
			}
		}

		bool stripPreviewText(BString &buf)
		{
			const char *pBuf = buf.c_str();
			const char *pEnd = buf.c_str() + buf.size();
			const char *eLine;
			int wrote {-1};
			int reply {0};
			uint size;
			BString result;

			while((size = getline(pBuf, pEnd)))
			{
				eLine = pBuf + size - 1;
				if(size >= 3 && *pBuf == '-' && *(pBuf + 1) == '-')
					break;
				if (!reply && (((size >= 2) && isspace(*eLine) && *(eLine - 1) == ':')
						|| ((size >= 3) && isspace(*eLine) && isspace(*(eLine - 1)) && *(eLine - 2) == ':')))
					wrote = result.size();
				if(*pBuf != '>')
					result.add(pBuf, size);
				else
				{
					if (!reply)
						reply = result.size();
				}
//				if (!reply && wrote >=0 && !isspace(*pBuf))
//					wrote = -1;
				pBuf += size;
			}

			if (wrote >= 0)
			{
				buf.clear();
				if (wrote > 0)
					buf.add(result.c_str(), wrote);
				buf.add(result.c_str() + reply, result.size() - reply);
				trimLRText(buf);
			}
			else if(result.size())
			{
				if (result.size() == buf.size()) {
					return false;
				} else {
					trimLRText(result);
					buf = std::move(result);
				}
			}
			return true;
		}

		void getTagParamPos(const char *s, const char* e, size_t& start, size_t& end)
		{
			const char* pos = s;
			bool assign = false;
			char quote = 0;
			end = 0;
			start = 0;

			while(pos < e)
			{
				if (!start)
				{
					if(*pos == '=')
					{
						assign = true;
						pos++;
					}
					if (assign && !isspace(*pos))
					{
						start = pos - s;
						if (*pos == '\"' || *pos == '\'' || *pos == '`' )
						{
							quote = *pos;
							start += 1;
						}
					}
				}
				else
				{
					if (quote)
					{
						if (*pos == quote)
						{
							end = pos - s;
							break;
						}
						if (*pos == '/' || *pos == '>')
							break;
					}
					else
					{
						if (isspace(*pos) || *pos == '/' || *pos == '>')
						{
							end = pos - s;
							break;
						}
					}
				}
				pos++;
			}
		}

		void getTagParamValuePos(const char *s, const char* e, size_t& start, size_t& end)
		{
			const char* pos = s;
			bool colon = false;
			end = 0;
			start = 0;

			while(pos < e)
			{
				if (!start)
				{
					if(*pos == ':')
					{
						colon = true;
						pos++;
					}
					if (colon && !isspace(*pos))
						start = pos - s;
				}
				else
				{
					if (isspace(*pos))
						break;
				}
				pos++;
			}
			end = pos - s;
		}

		bool findTagParamValue(BString& data, const std::string& param, const std::string& val, std::vector<std::tuple<const char*, size_t, std::string>>& vPos)
		{
			const char* START = data.c_str();
			const char* END = START + data.size();
			const char* pos = START;
			const char* old = pos;
			bool found = false;
			size_t sTag;
			size_t eTag;
			size_t sVal;
			size_t eVal;

			while (pos && pos < END)
			{
				if ((pos = fl::utils::strnstr(pos, END - pos, param.c_str(), param.size())))
				{
					pos += param.size();
					fl::utils::getTagParamPos(pos, END, sTag, eTag);
					if(sTag && eTag)
					{
						old = pos;
						if ((pos = fl::utils::strnstr(pos + sTag, eTag - sTag, val.c_str(), val.size())))
						{
							pos += val.size();
							fl::utils::getTagParamValuePos(pos, old + eTag, sVal, eVal);
							if (sVal && eVal)
							{
								std::string cid;
								cid.append("<");
								cid.append(pos + sVal, eVal - sVal);
								cid.append(">");
								vPos.emplace_back(std::make_tuple(old + sTag, eTag - sTag, std::move(cid)));
								pos = old + eTag;
								found = true;
							}
						}
						else
							pos = old;
					}
				}
				else
					return found;
			}
			return found;
		}

	};
};
