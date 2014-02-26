///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Buffer class declaration
///////////////////////////////////////////////////////////////////////////////

#include "buffer.hpp"

using namespace fl::utils;

Buffer::Buffer(const TSize reserved)
	: _begin(NULL), _end(NULL), _readPos(NULL), _writePos(NULL)
{
	if (reserved)
		reserve(reserved);
}

Buffer::~Buffer()
{
	free(_begin);
}

void Buffer::reserve(const TSize newSize)
{
	if (newSize == 0) {
		free(_begin);
		_begin = NULL;
		_end = NULL;
		_readPos = NULL;
		_writePos = 0;
	} else {
		TDataPtr newData = static_cast<TDataPtr>(malloc(newSize + 1));
		if (!newData)
			throw Error("malloc failed");
		TSize writtenSize = 0;
		if (_writePos > _begin) // need copy
		{
			writtenSize = _writePos - _begin;
			if (writtenSize > newSize)
				writtenSize = newSize;
			memcpy(newData, _begin, writtenSize);
		}
		free(_begin);
		_begin = newData;
		_end = _begin + newSize;
		_readPos = _begin;
		_writePos = _begin + writtenSize;
	}
}

inline void Buffer::_fit(const TSize size)
{
	if ((_writePos + size) < _end)
		return;
	TSize newSize = (_end - _begin) + size + 1;
	reserve(newSize);
}

Buffer::TDataPtr Buffer::reserveBuffer(const TSize size)
{
	_fit(size);
	TDataPtr curWritePos = _writePos;
	_writePos += size;
	return curWritePos;
}

Buffer::TDataPtr Buffer::mapBuffer(const TSize size)
{
	if ((_readPos + size) > _writePos)
		throw Error("Read out of range");
	TDataPtr curReadPos = _readPos;
	_readPos += size;
	return curReadPos;
}

void Buffer::skip(const TSize size)
{
	if ((_readPos + size) > _writePos)
		throw Error("Read out of range");
	_readPos += size;
}

void Buffer::add(const std::string &value)
{
	TSize size = value.size();
	_fit(sizeof(size) + size);
	memcpy(_writePos, &size, sizeof(size));
	_writePos += sizeof(size);
	memcpy(_writePos, value.c_str(), size);
	_writePos += size;
}

void Buffer::get(std::string &value)
{
	TSize size = value.size();
	if ((_readPos + sizeof(size) + size) > _writePos)
		throw Error("Read out of range");
	memcpy(&size, _readPos, sizeof(size));
	_readPos += sizeof(size);
	value.assign((char*)_readPos, size);
	_readPos += size;
}

void Buffer::add(const void *data, const TSize size)
{
	_fit(size);
	memcpy(_writePos, data, size);
	_writePos += size;
}

void Buffer::get(void *data, const TSize size)
{
	if ((_readPos + size) > _writePos)
		throw Error("Read out of range");
	memcpy(data, _readPos, size);
	_readPos += size;
}
