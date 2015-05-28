#pragma once
#ifndef __FL_SHA256_HPP
#define	__FL_SHA256_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: SHA-256 wrapper classes
///////////////////////////////////////////////////////////////////////////////

#include <openssl/sha.h>
#include <string>
#include "bstring.hpp"

namespace fl {
	namespace crypto {
		using fl::strings::BString;
		
		const size_t SHA256_HEX_SIZE = SHA256_DIGEST_LENGTH * 2;
		class SHA256Holder
		{
		public:
			SHA256Holder();
			SHA256Holder(const std::string &data);
			SHA256Holder(const BString &data);
			using TSHADigestPtr = uint8_t *;
			SHA256Holder(const TSHADigestPtr bytes, const size_t size);
			void setHex(const char *hex, size_t len = SHA256_HEX_SIZE);
			uint8_t const * bytes() const
			{
				return _bytes;
			}
			const size_t size() const
			{
				return SHA256_DIGEST_LENGTH;
			}
			bool operator!=(const std::string &binData) const;
			bool operator==(const std::string &binData) const;
			BString getBString() const;
			void toBString(BString &dst) const;
		private:
			uint8_t _bytes[SHA256_DIGEST_LENGTH];
		};
	}
};

#endif	// __FL_SHA256_HPP
