///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: SHA-1 wrapper classes
///////////////////////////////////////////////////////////////////////////////

#include <cstring>
#include "sha1.hpp"
#include "util.hpp"
#include "log.hpp"

using namespace fl::crypto;


SHA1Holder::SHA1Holder()
{
	bzero(_bytes, SHA1_BINARY_SIZE);
}

void SHA1Holder::clear()
{
	bzero(_bytes, SHA1_BINARY_SIZE);
}

SHA1Holder::SHA1Holder(const char *textSHA1, const size_t size)
{
	if (size != SHA1_HEX_SIZE)
		throw SHA1Exeption("Bad textSHA1 size");

	fl::utils::hex2Binary(textSHA1, _bytes, SHA1_BINARY_SIZE);
}

void SHA1Holder::setHex(const char *textSHA1, const size_t size)
{
	if (size != SHA1_HEX_SIZE)
		throw SHA1Exeption("Bad textSHA1 size");

	fl::utils::hex2Binary(textSHA1, _bytes, SHA1_BINARY_SIZE);
}

SHA1Holder::SHA1Holder(const TBinaryPtr bytes, const size_t size)
{
	if (size != SHA1_BINARY_SIZE)
		throw SHA1Exeption("Bad binarySHA1 size");
	memcpy(_bytes, bytes, size);
}

void SHA1Holder::setBinary(const TBinaryPtr bytes, const size_t size)
{
	if (size != SHA1_BINARY_SIZE)
		throw SHA1Exeption("Bad binarySHA1 size");
	memcpy(_bytes, bytes, size);	
}

bool SHA1Holder::operator==(const SHA1Holder &a) const
{
	return !memcmp(_bytes, a._bytes, SHA1_BINARY_SIZE);
}

bool SHA1Holder::operator!=(const SHA1Holder &a) const
{
	return memcmp(_bytes, a._bytes, SHA1_BINARY_SIZE);
}

bool SHA1Holder::operator<(const SHA1Holder &a) const
{
	return memcmp(_bytes, a._bytes, SHA1_BINARY_SIZE) < 0;
}

SHA1Holder::SHA1Holder(const fl::utils::Buffer &buf)
{
	SHA1(buf.begin(), buf.writtenSize(), _bytes);
}

SHA1Holder::SHA1Holder(const fl::strings::BString &str)
{
	SHA1((unsigned char*)str.c_str(), str.size(), _bytes);
}

SHA1Holder::SHA1Holder(fl::fs::File &file, const size_t readSize, fl::utils::BString &buf)
{
	if (!fromFile(file, readSize, buf)) {
		throw SHA1Exeption("SHA1Holder can't calc SHA1 from file");
	}
}

bool SHA1Holder::fromFile(fl::fs::File &file, const size_t readSize, fl::utils::BString &buf)
{
	if (buf.reserved() < 1) {
		log::Fatal::L("Buffer can't have zero size");
		return false;
	}
	
	SHA1Builder builder;
	auto leftSize = readSize;
	while (leftSize > 0) {
		buf.clear();
		auto chunkSize = buf.reserved() - 1;
		if (chunkSize > leftSize) {
			chunkSize = leftSize;
		}
		if (file.read(buf.reserveBuffer(chunkSize), chunkSize) != chunkSize) {
			log::Error::L("SHA1Holder can't read %u chunk\n", chunkSize);
			return false;
		}
		builder.update(buf.c_str(), buf.size());
		leftSize -= chunkSize;
	}
	builder.finish(*this);
	return true;
}


BString SHA1Holder::getBString() const
{
	BString buf(SHA1_HEX_SIZE + 1);
	toBString(buf);
	return buf;
}

void SHA1Holder::toBString(BString &dst) const
{
	static const char hexChars[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	char *buf = dst.reserveBuffer(SHA1_HEX_SIZE);
	for (size_t i = 0; i < SHA1_BINARY_SIZE; i++) 
	{
		*buf = hexChars[(_bytes[i] & 0xF0) >> 4];
		buf++;
		*buf = hexChars[_bytes[i] & 0x0F];
		buf++;
	}
	*buf = 0; 
}

size_t SHA1Holder::crc64() const
{
	return fl::utils::getCheckSum64((const char*)_bytes, SHA1_BINARY_SIZE);
}

uint16_t SHA1Holder::getUINT16() const
{
	uint16_t val = _bytes[0];
	val <<= 8;
	val |= _bytes[1];
	return val;
}

bool SHA1Holder::empty() const
{
	for (uint8_t i = 0; i < SHA1_BINARY_SIZE; i++) {
		if (_bytes[i] != 0) {
			return false;
		}
	}
	return true;
}

SHA1Builder::SHA1Builder()
	: updated(false)
{
	if (!SHA1_Init(&_ctx)) {
		throw SHA1Exeption("Can't initialize sha1 ctx");
	}
}

void SHA1Builder::clear()
{
	if (!SHA1_Init(&_ctx)) {
		throw SHA1Exeption("Can't initialize sha1 ctx");
	}
	updated = false;
}

void SHA1Builder::update( const void *data, unsigned long len)
{
	if (!SHA1_Update(&_ctx, data, len)) {
		throw SHA1Exeption("Can't update sha1 ctx");
	}
	updated = true;
}

void SHA1Builder::finish(SHA1Holder &sha1)
{
	if (updated) {
		if (!SHA1_Final(sha1._bytes, &_ctx)) {
			throw SHA1Exeption("Can't finish sha1 ctx");
		}
		updated = false;
	} else {
		sha1.clear();
	}
}
