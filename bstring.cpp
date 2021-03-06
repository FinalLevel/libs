///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Buffered string class implementation
///////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <cstring>
#include <stdlib.h>
#include <cstdio>

#include "bstring.hpp"
#include "buffer.hpp"
using namespace fl::strings;

void BString::reserve(const TSize newSize)
{
	if (newSize == 0) {
		free(_data);
		_data = NULL;
		_reserved = 0;
		_size = 0;
	} else {
		TDataPtr newData = static_cast<TDataPtr>(malloc(newSize + 1));
		if (!newData)
			throw BString::Error("malloc failed");
		if (_size > 0) // need copy
		{
			if (_size > newSize)
				_size = newSize;
			memcpy(newData, _data, _size);
		}
		free(_data);
		_data = newData;
		_reserved = newSize;
		_data[_size] = 0;
	}
}

inline void BString::_fit(const TSize size)
{
	if (_size + size < _reserved)
		return;
	reserve(_size + size + 1);
}

BString::BString(const TSize reserved)
	: _size(0), _reserved(reserved), _data(NULL)
{
	reserve(_reserved);
}

BString::BString(const char *str)
	: _size(0), _reserved(0), _data(NULL)
{
	add(str, strlen(str));
}

BString::~BString()
{
	free(_data);
}

BString::BString(BString &&moveFrom) noexcept
	: _size(moveFrom._size), _reserved(moveFrom._reserved), _data(moveFrom._data)
{
	moveFrom._data = NULL;
	moveFrom._reserved = 0;
	moveFrom._size = 0;
}

BString& BString::operator=(BString &&moveFrom) noexcept
{
	std::swap(moveFrom._size, _size);
	std::swap(moveFrom._reserved, _reserved);
	std::swap(moveFrom._data, _data);
	return *this;
}

BString& BString::operator=(fl::utils::Buffer &&moveFrom) noexcept
{
	_size = moveFrom.writtenSize();
	_reserved = moveFrom.reserved();
	_data = (TDataPtr)moveFrom.release();
	return *this;
}

BString& BString::operator=(const std::string &data)
{
	clear();
	add(data.c_str(), data.size());
	return *this;
}

inline bool BString::_reserveForSprintf(const int sprintfRes, const TSize leftSpace)
{
	if ((sprintfRes >= 0) && (static_cast<TSize>(sprintfRes) < leftSpace))
		return true;

	int newReservedSize = _reserved;
	if (sprintfRes > 0) // vsnprintf returned how many symbols it needed
		newReservedSize += (sprintfRes + 1);	
	else
		newReservedSize *= 2;
	reserve(newReservedSize);
	return false;
}

inline bool BString::_sprintfAdd(const char *fmt, TSize &charsAdded, va_list args)
{
	TSize leftSpace = _reserved - _size;
	charsAdded = vsnprintf (_data + _size, leftSpace, fmt, args);
	if (_reserveForSprintf(charsAdded, leftSpace)) {
		_size += charsAdded;
		return true;
	}
	else
		return false;
}

BString::TSize BString::sprintfAdd(const char *fmt, ...)
{
	va_list ap;
	TSize charsAdded = 0;
  while (1) {
	  va_start(ap, fmt);
		bool result = _sprintfAdd(fmt, charsAdded, ap);
		va_end(ap);
		if (result)
			return charsAdded;
	}
}

BString::TSize BString::sprintfSet(const char *fmt, ...)
{
	_size = 0;
	
	TSize charsAdded = 0;
	va_list ap;
  while (1) {
	  va_start(ap, fmt);
		bool result = _sprintfAdd(fmt, charsAdded, ap);
		va_end(ap);
		if (result)
			return charsAdded;
	}
}


const bool BString::operator==(const BString &str) const
{
	if (str._size != _size)
		return false;
	return !memcmp(_data, str._data, _size);
}


const bool BString::operator!=(const std::string &str) const
{
	if (str.size() != (size_t)_size)
		return true;
	return memcmp(_data, str.c_str(), _size);
}

const bool BString::operator==(const std::string &str) const
{
	if (str.size() != (size_t)_size)
		return false;
	return !memcmp(_data, str.c_str(), _size);
}

const bool BString::operator==(const char *compareWith) const
{
	TSize strLen = strlen(compareWith);
	if (strLen != _size)
		return false;
	return !memcmp(_data, compareWith, _size);
}

BString &BString::operator<<(const char *str)
{
	TSize len = strlen(str);
	add(str, len);
	return *this;
}

BString &BString::operator<<(const unsigned int num)
{
	sprintfAdd("%u", num);
	return *this;
}


BString &BString::operator<<(const uint64_t num)
{
	sprintfAdd("%llu", num);
	return *this;	
}

BString &BString::operator<<(const int64_t num)
{
	sprintfAdd("%lld", num);
	return *this;	
}


BString &BString::operator<<(const int num)
{
	sprintfAdd("%d", num);
	return *this;
}

BString &BString::operator<<(const double num)
{
	sprintfAdd("%f", num);
	return *this;
}

BString &BString::operator<<(const std::string &str)
{
	add(str.c_str(), str.size());
	return *this;
}

BString &BString::operator<<(const BString &str)
{
	add(str.c_str(), str.size());
	return *this;
}

BString &BString::operator<<(const char ch)
{
	_fit(1);
	*(_data + _size) = ch;
	_size++;
	*(_data + _size) = 0;
	return *this;
}

void BString::add(const char *str, TSize len)
{
	_fit(len);
	memcpy(_data + _size, str, len);
	_size += len;
	_data[_size] = 0;
}

void BString::addJSONEscapedUTF8(const char *str, TSize len)
{
	static const char hexDigits[] = "0123456789ABCDEF";
	static const char escape[256] = {
#define Z16 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
		//0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
		'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'b', 't', 'n', 'u', 'f', 'r', 'u', 'u', // 00
		'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', // 10
		  0,   0, '"',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 20
		Z16, Z16,																		// 30~4F
		  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,'\\',   0,   0,   0, // 50
		Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16								// 60~FF
#undef Z16
	};
	const char *pstr = str;
	const char *strEnd = str + len;
	TSize escapedSize = 0;
	while (pstr < strEnd) {
		unsigned char ch = *pstr;
		unsigned char esc = escape[ch];
		if (esc)  {
			if (esc == 'u') {
				escapedSize += 6;
			} else {
				escapedSize += 2;
			}
		}
		pstr++;
	}
	if (escapedSize == 0) {
		add(str, len);
		return;
	}
	TSize startSize = _size;
	char *pBufStart = reserveBuffer(len + escapedSize + 1);
	char *pBuf = pBufStart;
	pstr = str;
	strEnd = str + len;
	while (pstr < strEnd) {
		unsigned char ch = *pstr;
		unsigned char esc = escape[ch];
		if (esc)  {
			*pBuf = '\\';
			pBuf++;
			*pBuf = esc;
			pBuf++;
			if (esc == 'u') {
				*pBuf = '0';
				pBuf++;
				*pBuf = '0';
				pBuf++;
				*pBuf = hexDigits[ch >> 4];
				pBuf++;
				*pBuf = hexDigits[ch & 0xF];
				pBuf++;
			}
		} else {
			*pBuf = ch;
			pBuf++;
		}
		pstr++;
	}
	trim(startSize + (pBuf - pBufStart));
}

BString::TDataPtr BString::reserveBuffer(const TSize size)
{
	TSize oldSize = _size;
	_fit(size);
	_size += size;
	return (_data + oldSize);
}

void BString::trim(TSize size)
{
	if (size > _size)
		return;
	_size = size;
	_data[_size] = 0;
}

void BString::trimLast()
{
	if (_size > 0)
	{
		_size--;
		_data[_size] = 0;
	}
}

void BString::trimLastSpaces()
{
	while (_size > 0 && isspace(_data[_size - 1])) {
		_size--;
		_data[_size] = 0;
	}
}

BString::TDataPtr BString::release()
{
	TDataPtr data = _data;
	_data = NULL;
	_reserved = 0;
	_size = 0;
	return data;
}
