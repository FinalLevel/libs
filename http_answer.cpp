///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Http answer utility class implementation
///////////////////////////////////////////////////////////////////////////////

#include "http_answer.hpp"
#include "log.hpp"

using namespace fl::http;

HttpAnswer::HttpAnswer(BString &buf, const std::string &httpStatus, const char *contentType, const std::string& headers)
	: _buf(buf)
{
	buf << httpStatus;
	buf << "Content-Type: " << contentType << "\r\n" << headers;
	_contentLengthStart = buf.size();
	buf << "Content-Length: 0000000000\r\n\r\n";
	_headersEnd = buf.size();
}

void HttpAnswer::addHeaders(const std::string& headers)
{
	_buf.trim(_buf.size() - 2); // remove end \r\n
	_buf << headers << "\r\n";
	_headersEnd = _buf.size();
}

void HttpAnswer::setContentLength()
{
	auto contentLength = _buf.size() - _headersEnd;
	char *pDigitsStart = _buf.data() + _contentLengthStart + sizeof("Content-Length:");
	auto res = snprintf(pDigitsStart, 11, "%010u", contentLength);
	if (res != 10) {
		log::Error::L("Cannot set content length: content length %u too big, result: %d\n", contentLength, res);
		throw std::exception();
	}
	*(pDigitsStart + res) = '\r';
}