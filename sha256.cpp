///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: SHA-256 wrapper classes implementation
///////////////////////////////////////////////////////////////////////////////


#include "sha256.hpp"
#include "util.hpp"
#include "log.hpp"
#include <cstring>

using namespace fl::crypto;

SHA256Holder::SHA256Holder()
{
	bzero(_bytes, SHA256_DIGEST_LENGTH);
}

SHA256Holder::SHA256Holder(const std::string &data)
{
	SHA256(reinterpret_cast<const unsigned char*>(data.c_str()), data.size(), _bytes);
}

SHA256Holder::SHA256Holder(const BString &data)
{
	SHA256(reinterpret_cast<const unsigned char*>(data.c_str()), data.size(), _bytes);
}

SHA256Holder::SHA256Holder(const TSHADigestPtr bytes, const size_t size)
{
	if (size != SHA256_DIGEST_LENGTH) {
		log::Fatal::L("SHA256 digest size should be equal %u, got %u\n", SHA256_DIGEST_LENGTH, size);
		throw std::exception();
	}
	memcpy(_bytes, bytes, size);
}

bool SHA256Holder::operator!=(const std::string &binData) const
{
	return !operator==(binData);
}

bool SHA256Holder::operator==(const std::string &binData) const
{
	if (binData.size() != SHA256_DIGEST_LENGTH) {
		return true;
	}
	return memcmp(_bytes, binData.c_str(), SHA256_DIGEST_LENGTH) == 0;
}

void SHA256Holder::setHex(const char *hex, size_t len)
{
	fl::utils::hex2Binary(hex, _bytes, SHA256_DIGEST_LENGTH);
}

BString SHA256Holder::getBString() const
{
	BString buf(SHA256_HEX_SIZE + 1);
	toBString(buf);
	return buf;	
}

void SHA256Holder::toBString(BString &dst) const
{
	static const char hexChars[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	char *buf = dst.reserveBuffer(SHA256_HEX_SIZE);
	for (size_t i = 0; i < SHA256_DIGEST_LENGTH; i++) 
	{
		*buf = hexChars[(_bytes[i] & 0xF0) >> 4];
		buf++;
		*buf = hexChars[_bytes[i] & 0x0F];
		buf++;
	}
	*buf = 0; 	
}
