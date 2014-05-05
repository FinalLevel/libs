///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Http answer utility class implementation
///////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include "http_answer.hpp"
#include "log.hpp"

using namespace fl::http;

HttpAnswer::HttpAnswer(BString &buf, const std::string &httpStatus, const char *contentType, const bool isKeepAlive, 
	const std::string& headers)
	: _buf(buf)
{
	buf.clear();
	buf << httpStatus;
	buf << "Content-Type: " << contentType << "\r\n" << headers;
	if (isKeepAlive)
		_buf << "Connection: Keep-Alive\r\n";
	else
		_buf << "Connection: Close\r\n";
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
	setContentLength(_buf.size() - _headersEnd);
}

void HttpAnswer::setContentLength(const uint32_t contentLength)
{
	char *pDigitsStart = _buf.data() + _contentLengthStart + sizeof("Content-Length:");
	auto res = snprintf(pDigitsStart, 11, "%010u", contentLength);
	if (res != 10) {
		log::Error::L("Cannot set content length: content length %u too big, result: %d\n", contentLength, res);
		throw std::exception();
	}
	*(pDigitsStart + res) = '\r';
}


const char *MimeType::_MIME_TYPES[E_MAX] = {
	"application/octet-stream",
	"image/jpeg",
	"image/gif",
	"image/png",
};

MimeType::TExtMimeTypeMap MimeType::_mimeTypes = {
	{"jpeg", E_JPEG}, 
	{"jpg", E_JPEG},
	{"gif", E_GIF},
	{"png", E_PNG},
};

MimeType::EMimeType MimeType::getMimeTypeFromFileName(const std::string &fileName)
{
	const char *endFile = fileName.c_str() + fileName.size();
	const char *ext = endFile;
	while (ext > fileName.c_str()) {
		if (*ext == '.') {
			ext++;
			size_t len = endFile - ext;
			if (len < 1 || len > MAX_EXT_LENGTH)
				return E_UNKNOWN;
			std::string extStr(ext, len);
			std::transform(extStr.begin(), extStr.end(), extStr.begin(), ::tolower);
			auto f = _mimeTypes.find(extStr);
			if (f == _mimeTypes.end())
				return E_UNKNOWN;
			else 
				return f->second;	
		}
		ext--;
	}
	return E_UNKNOWN;
}
