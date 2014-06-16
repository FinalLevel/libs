#pragma once
#ifndef __FL_SHA1_HPP
#define	__FL_SHA1_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: SHA-1 wrapper classes
///////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <cstddef>
#include "exception.hpp"
#include "buffer.hpp"


namespace fl {
	namespace crypto {
		const size_t SHA1_BINARY_SIZE = 20;
		const size_t SHA1_HEX_SIZE = SHA1_BINARY_SIZE * 2;
		
		class SHA1Exeption : fl::exceptions::Error
		{
		public:
			SHA1Exeption(const char *what) 
				: Error(what)
			{
			}
		};

		class SHA1Holder
		{
		public:
			typedef uint8_t* TBinaryPtr;
			
			SHA1Holder();
			SHA1Holder(const char *textSHA1, const size_t size);
			SHA1Holder(const TBinaryPtr bytes, const size_t size);
			SHA1Holder(fl::utils::Buffer &buf);
			bool operator==(const SHA1Holder &a);
			
			void setHex(const char *textSHA1, const size_t size);
		private:
			uint8_t _bytes[SHA1_BINARY_SIZE];
		};
	};
};

#endif	// __FL_SHA1_HPP
