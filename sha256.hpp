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

namespace fl {
	namespace crypto {
		
		class SHA256Holder
		{
		public:
			SHA256Holder();
			SHA256Holder(const std::string &data);
			uint8_t const * bytes() const
			{
				return _bytes;
			}
			const size_t size() const
			{
				return SHA256_DIGEST_LENGTH;
			}
		private:
			uint8_t _bytes[SHA256_DIGEST_LENGTH];
		};
	}
};

#endif	// __FL_SHA256_HPP
