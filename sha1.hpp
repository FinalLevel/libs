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
#include "bstring.hpp"


namespace fl {
	namespace crypto {
		using fl::strings::BString;
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
			SHA1Holder(const fl::utils::Buffer &buf);
			SHA1Holder(const fl::strings::BString &str);
			bool operator==(const SHA1Holder &a) const;
			
			void setHex(const char *textSHA1, const size_t size);
			void toBString(BString &dst) const;
			BString getBString() const;
			size_t crc64() const;
			uint16_t getUINT16() const;
			bool empty() const;
		private:
			uint8_t _bytes[SHA1_BINARY_SIZE];
		};
	};
};

namespace std
{
	using fl::crypto::SHA1Holder;
	
	template<> struct hash<SHA1Holder> 
	{
		size_t operator()(const SHA1Holder& val) const
		{
			return val.crc64();
		}
	};	
};

#endif	// __FL_SHA1_HPP
