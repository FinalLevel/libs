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

using namespace fl::strings;

inline void BString::_reserve(const TSize newSize)
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
			memcpy(newData, _data, _size);
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
	_reserve(_size + size + 1);
}

BString::BString(const TSize reserved)
	: _size(0), _reserved(reserved), _data(NULL)
{
	_reserve(_reserved);
}

BString::~BString()
{
	free(_data);
}

BString::BString(BString &&moveFrom)
	: _size(moveFrom._size), _reserved(moveFrom._reserved), _data(moveFrom._data)
{
	moveFrom._data = NULL;
	moveFrom._reserved = 0;
	moveFrom._size = 0;
}

BString& BString::operator=(BString &&moveFrom)
{
	std::swap(moveFrom._size, _size);
	std::swap(moveFrom._reserved, _reserved);
	std::swap(moveFrom._data, _data);
	return *this;
}

inline bool BString::_reserveForSprintf(const int sprintfRes, const TSize leftSpace)
{
	if ((sprintfRes >= 0) && (sprintfRes < leftSpace))
		return true;

	int newReservedSize = _reserved;
	if (sprintfRes > 0) // vsnprintf returned how many symbols it needed
		newReservedSize += (sprintfRes + 1);	
	else
		newReservedSize *= 2;
	_reserve(newReservedSize);
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

BString &BString::operator<<(const int num)
{
	sprintfAdd("%d", num);
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
