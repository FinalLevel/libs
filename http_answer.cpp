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

const std::string HttpAnswer::CONNECTION_KEEP_ALIVE = "Connection: Keep-Alive\r\n";
const std::string HttpAnswer::CONNECTION_CLOSE = "Connection: Close\r\n";

HttpAnswer::HttpAnswer(BString &buf, const std::string &httpStatus, const char *contentType, const bool isKeepAlive, 
	const std::string& headers)
	: _buf(buf)
{
	buf.clear();
	buf << httpStatus;
	buf << "Content-Type: " << contentType << "\r\n" << headers;
	if (isKeepAlive)
		_buf << CONNECTION_KEEP_ALIVE;
	else
		_buf << CONNECTION_CLOSE;
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

void HttpAnswer::addLastModified(const time_t unixTime)
{
	_buf.trim(_buf.size() - 2); // remove end \r\n
	formLastModified(unixTime, _buf);
	_buf << "\r\n";
	_headersEnd = _buf.size();
}

void HttpAnswer::setContentLength()
{
	setContentLength(_buf.size() - _headersEnd);
}


void HttpAnswer::add(const char *data, const size_t size)
{
	_buf.add(data, size);
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
	"text/plain",
	"audio/mpeg",
};

MimeType::TExtMimeTypeMap MimeType::_mimeTypes = {
	{"jpeg", E_JPEG}, 
	{"jpg", E_JPEG},
	{"gif", E_GIF},
	{"png", E_PNG},
	{"txt", E_TXT},
	{"mp3", E_MP3},
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

MimeType::EMimeType MimeType::getMimeTypeFromExt(const char *ext, const size_t extLen)
{
	std::string extStr(ext, extLen);
	std::transform(extStr.begin(), extStr.end(), extStr.begin(), ::tolower);
	auto f = _mimeTypes.find(extStr);
	if (f == _mimeTypes.end())
		return E_UNKNOWN;
	else 
		return f->second;	
}

void HttpAnswer::formLastModified(const time_t unixTime, BString &buf)
{
	static const char *DAY_NAMES[]={"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  static const char *MONTH_NAMES[]={"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", 
		"Dec"};
	struct tm result;
  struct tm *timeStruct = gmtime_r(&unixTime, &result);
  buf.sprintfAdd("Last-Modified: %s, %02d %s %04d %02d:%02d:%02d GMT\r\n", 
		DAY_NAMES[timeStruct->tm_wday], timeStruct->tm_mday, MONTH_NAMES[timeStruct->tm_mon], timeStruct->tm_year + 1900, 
		timeStruct->tm_hour, timeStruct->tm_min, timeStruct->tm_sec);	
}

