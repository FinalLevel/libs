///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Mime types wrapper class implementation
///////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include "mime_type.hpp"

using namespace fl::http;

const char *MimeType::_MIME_TYPES[E_MAX] = {
	"application/octet-stream",
	"image/jpeg",
	"image/gif",
	"image/png",
	"text/plain",
	"audio/mpeg",
	"audio/x-flac",
	"audio/ogg",
	"audio/mp4",
	"audio/x-ms-wma",
	"audio/opus",
	"audio/x-alac",
	"text/html",
	"text/calendar",
};

MimeType::TExtMimeTypeMap MimeType::_mimeTypes = {
	{"jpeg", E_JPEG}, 
	{"jpg", E_JPEG},
	{"gif", E_GIF},
	{"png", E_PNG},
	{"txt", E_TXT},
	{"mp3", E_MP3},
	{"flac", E_FLAC},
	{"ogg", E_VORBIS},
	{"oga", E_VORBIS},
	{"m4a", E_M4A},
	{"wma", E_WMA},
	{"opus", E_OPUS},
	{"alac", E_ALAC},
	{"html", E_HTML},
	{"htm", E_HTML},
	{"ics", E_CALENDAR},
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

