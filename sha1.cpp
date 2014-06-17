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

#include <openssl/sha.h>

using namespace fl::crypto;


SHA1Holder::SHA1Holder()
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

bool SHA1Holder::operator==(const SHA1Holder &a)
{
	return !memcmp(_bytes, a._bytes, SHA1_BINARY_SIZE);
}

SHA1Holder::SHA1Holder(fl::utils::Buffer &buf)
{
	SHA1(buf.begin(), buf.writtenSize(), _bytes);
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
