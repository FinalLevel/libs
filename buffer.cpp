///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Buffer class declaration
///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
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

Buffer::TDataPtr Buffer::release() noexcept
{
	TDataPtr begin = _begin;
	_begin = NULL;
	_end = NULL;
	_readPos = NULL;
	_writePos = NULL;
	return begin;
}

Buffer::Buffer(BString &&str)
{
	TSize reserved = str.reserved();
	TSize size = str.size();
	_begin =(TDataPtr)str.release();
	_end =  _begin + reserved;
	_readPos = _begin;
	_writePos = _begin + size;
}

Buffer& Buffer::operator=(BString &&str)
{
	free(_begin);
	
	TSize reserved = str.reserved();
	TSize size = str.size();
	_begin =(TDataPtr)str.release();
	_end =  _begin + reserved;
	_readPos = _begin;
	_writePos = _begin + size;
	return *this;
}

void Buffer::reserve(const TSize newSize)
{
	if (newSize == 0) {
		free(_begin);
		_begin = NULL;
		_end = NULL;
		_readPos = NULL;
		_writePos = NULL;
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
		TSize readed = readPos();
		if (readed > writtenSize) {
			readed = writtenSize;
		}
		free(_begin);
		_begin = newData;
		_end = _begin + newSize;
		_readPos = _begin + readed;
		_writePos = _begin + writtenSize;
	}
}

void Buffer::_fit(const TSize size)
{
	if ((_writePos + size) < _end)
		return;
	TSize newSize = (_end - _begin) + size + 1;
	reserve(newSize);
}


Buffer::TSize Buffer::addSpace(const TSize size)
{
	_fit(size);
	TDataPtr curWritePos = _writePos;
	_writePos += size;
	return (curWritePos - _begin);
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


void Buffer::truncate(const TSize seek)
{
	if (seek > writtenSize())
		throw Error("Seek out of range");
	_writePos = _begin + seek;
	if (_readPos > _writePos)
		_readPos = _writePos;
}

void Buffer::seekReadPos(const TSize seek)
{
	TDataPtr newPos = _begin + seek;
	if (newPos > _writePos)
		throw Error("Seek out of range");
	_readPos = newPos;
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

void Buffer::add(const BString &value)
{
	TSize size = value.size();
	_fit(sizeof(size) + size);
	memcpy(_writePos, &size, sizeof(size));
	_writePos += sizeof(size);
	memcpy(_writePos, value.c_str(), size);
	_writePos += size;	
}

void Buffer::get(BString &value)
{
	TSize size {0};
	if ((_readPos + sizeof(size)) > _writePos) {
		throw Error("Read out of range");
	}
	
	memcpy(&size, _readPos, sizeof(size));
	_readPos += sizeof(size);
	if ((_readPos + size) > _writePos) {
		throw Error("Read out of range");
	}
	
	value.clear();
	value.add((char*)_readPos, size);
	_readPos += size;
}

void Buffer::get(std::string &value)
{
	TSize size {0};
	if ((_readPos + sizeof(size)) > _writePos)
		throw Error("Read out of range");
	
	memcpy(&size, _readPos, sizeof(size));
	_readPos += sizeof(size);
	if ((_readPos + size) > _writePos) {
		throw Error("Read out of range");
	}
	value.assign((char*)_readPos, size);
	_readPos += size;
}

void Buffer::add(const void *data, const TSize size)
{
	_fit(size);
	memcpy(_writePos, data, size);
	_writePos += size;
}

void Buffer::set(const TSize seek, const void *data, const TSize size)
{
	TDataPtr setPos = _begin + seek;
	if (setPos + size > _writePos) {
		throw Error("Read out of range");
	}
	memcpy(setPos, data, size);
}

void Buffer::get(void *data, const TSize size)
{
	if ((_readPos + size) > _writePos)
		throw Error("Read out of range");
	memcpy(data, _readPos, size);
	_readPos += size;
}
