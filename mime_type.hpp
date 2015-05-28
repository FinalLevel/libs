#pragma once
#ifndef __FL_HTTP_MIME_TYPE_HPP
#define	__FL_HTTP_MIME_TYPE_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Mime types wrapper class
///////////////////////////////////////////////////////////////////////////////

#include <map>
#include <string>

namespace fl {
	namespace http {
		
		class MimeType
		{
		public:
			enum EMimeType : uint8_t
			{
				E_UNKNOWN,
				E_JPEG,
				E_GIF,
				E_PNG,
				E_TXT,
				E_MP3,
				E_FLAC,
				E_VORBIS,
				E_M4A,
				E_WMA,
				E_OPUS,
				E_ALAC,
				E_HTML,
				E_CALENDAR,
				E_MULTIPART_MIXED,
				E_MULTIPART_ALTERNATIVE,
				E_MULTIPART_RELATED,
				E_MAX,
			};
			static const char *getMimeTypeStrFromFileName(const std::string &fileName)
			{
				return _MIME_TYPES[getMimeTypeFromFileName(fileName)];
			}
			static const char *getMimeTypeStr(const EMimeType type) 
			{
				return _MIME_TYPES[type];
			}
			static EMimeType getMimeTypeFromFileName(const std::string &fileName);
			static EMimeType getMimeTypeFromExt(const char *ext, const size_t extLen);
			static EMimeType getMimeTypeFromName(const std::string &mimeName);
			static bool isText(const EMimeType type)
			{
				if (type == E_HTML || type == E_TXT || type == E_CALENDAR) {
					return true;
				} else {
					return false;
				}
			}
		private:
			static const size_t MAX_EXT_LENGTH = 4;
			static const char *_MIME_TYPES[E_MAX];
			typedef std::map<std::string, EMimeType> TExtMimeTypeMap;
			static TExtMimeTypeMap _mimeTypes;
		};
	};
};

#endif	// __FL_HTTP_MIME_TYPE_HPP
