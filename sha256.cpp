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

void SHA256Holder::setHex(const char *hex, size_t len)
{
	fl::utils::hex2Binary(hex, _bytes, SHA256_DIGEST_LENGTH);
}
