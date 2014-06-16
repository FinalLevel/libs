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