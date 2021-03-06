#pragma once
#ifndef __FL_HTTP_ANSWER_HPP
#define	__FL_HTTP_ANSWER_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Http answer utility class
///////////////////////////////////////////////////////////////////////////////

#include "bstring.hpp"
#include "mime_type.hpp"

namespace fl {
	namespace http {
		using fl::strings::BString;
		
		const std::string CACHE_PREVENTING_HEADERS = "Cache-Control: no-cache, no-store, must-revalidate,\
 no-cache=Set-Cookie, max-age=0, proxy-revalidate\r\nExpires: Thu, 01 Jan 1970 00:00:00 GMT\r\nPragma: no-cache\r\n";
		
		const std::string HTTP_OK_STATUS = "HTTP/1.1 200 OK\r\n";
		
		class HttpAnswer
		{
		public:
			HttpAnswer(BString &buf, const std::string &httpStatus, const char *contentType, const bool isKeepAlive,
				const std::string& headers = std::string());
			void addHeaders(const std::string &headers);
			void addHeaders(const char *headers, const size_t length);
			void setContentLength();
			void setContentLength(const uint32_t contentLength);
			void addLastModified(const time_t unixTime);
			static void formLastModified(const time_t unixTime, BString &buf);
			BString::TSize headersEnd() const
			{
				return _headersEnd;
			}
			void add(const char *data, const size_t size);
			BString &buffer()
			{
				return _buf; 
			}
			static const std::string CONNECTION_KEEP_ALIVE;
			static const std::string CONNECTION_CLOSE;
		private:
			BString &_buf;
			BString::TSize _contentLengthStart;
			BString::TSize _headersEnd;
		};		
	};
};

#endif	// __FL_HTTP_ANSWER_HPP
