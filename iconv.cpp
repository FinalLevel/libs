///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Iconv library wrapper function implementations
///////////////////////////////////////////////////////////////////////////////

#include "iconv.hpp"
#include "log.hpp"
#include <iconv.h>
#include <map>

namespace fl {
	namespace iconv {
		struct ciLessLibC : public std::binary_function<std::string, std::string, bool> {
			bool operator()(const std::string &lhs, const std::string &rhs) const {
				return strcasecmp(lhs.c_str(), rhs.c_str()) < 0 ;
			}
		};
		using TCharsetMap = std::map<std::string, ECharset,ciLessLibC>;
		static const TCharsetMap CHARSET_MAP = {
			{ "utf-8", ECharset::UTF8 },
			{ "utf8", ECharset::UTF8 },
			{ "windows-1251", ECharset::WINDOWS1251 },
			{ "windows1251", ECharset::WINDOWS1251 },
			{ "cp1251", ECharset::WINDOWS1251 },
		};
		ECharset getCharsetId(const std::string &charset)
		{
			auto f = CHARSET_MAP.find(charset);
			if (f == CHARSET_MAP.end()) {
				return ECharset::UNKNOWN; 
			} else {
				return f->second;
			}
		}
		bool convert(const char *input, const size_t size, fl::strings::BString &result, const ECharset from, 
			const ECharset to)
		{
			return convert(input, size, result, CHARSETS[static_cast<uint32_t>(from)].name, to);
		}
		bool convert(const char *input, const size_t size, fl::strings::BString &result, const char *from, const ECharset to)
		{
			iconv_t iconvDescriptor;
			auto &toCharset = CHARSETS[static_cast<uint32_t>(to)];
			if ((iconvDescriptor = iconv_open(toCharset.name, from)) == (iconv_t) -1) {
				log::Error::L("Cannot open iconv table from %s to %s\n", from, toCharset.name);
				return false;
			}
			char *inputBuf = const_cast<char *>(input);
			size_t inputBufSize = size;
			size_t outputBufSize = (size * toCharset.avgSize) + 1;
			char *outputBuf = result.reserveBuffer(outputBufSize);

			while (inputBufSize > 0) {
				auto res = ::iconv(iconvDescriptor, &inputBuf, &inputBufSize, &outputBuf, &outputBufSize);
				if (res == 0) {
					break;
				} else if (res == static_cast<decltype(res)>(-1)) {
					if (errno == E2BIG) {
						result.trim(result.size() - outputBufSize);
						outputBufSize = (inputBufSize * toCharset.avgSize) + 1;
						outputBuf = result.reserveBuffer(outputBufSize);
						continue;
					} else if ((errno == EILSEQ) ||  (errno == EINVAL)) {
					} else {
						break;
					}
				}
				if ((inputBufSize > 0) && (outputBufSize > 0)) {
					*outputBuf = *inputBuf;
					outputBuf++;
					inputBuf++;
					inputBufSize--;
					outputBufSize--;
				}
			}
			iconv_close(iconvDescriptor);
			result.trim(result.size() - outputBufSize);
			return true;
		}
	};
};