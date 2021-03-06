#pragma once
#ifndef __FL_BUFFER_HPP
#define	__FL_BUFFER_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Buffer class declaration
///////////////////////////////////////////////////////////////////////////////


#include <string>
#include <cstring>
#include <memory>
#include "exception.hpp"
#include "bstring.hpp"

namespace fl {
	namespace utils {
		using fl::strings::BString;
		
		class Buffer
		{
		public:
			class Error : public fl::exceptions::Error
			{
			public:
				Error(const char *what)
					: fl::exceptions::Error(what)
				{
				}
			};			
			typedef uint32_t TSize;
			typedef uint8_t *TDataPtr;
			Buffer(const TSize reserved = 0);
			~Buffer();
			Buffer(const Buffer &) = delete;
			Buffer &operator=(const Buffer &str) = delete;
			
			Buffer(BString && str);
			Buffer(const BString &str) = delete;
			
			Buffer &operator=(const BString &str) = delete;
			Buffer &operator=(BString && str);
			
			template<class D> void add(const D &value)
			{
				_fit(sizeof(D));
				memcpy(_writePos, &value, sizeof(D));
				_writePos += sizeof(D);
			};
			void add(const std::string &value);
			void add(const BString &value);
			void add(const void *data, const TSize size);
			void set(const TSize seek, const void *data, const TSize size);
			
			template<class D> void get(D &value)
			{
				if ((_readPos + sizeof(D)) > _writePos)
					throw Error("Read out of range");
				memcpy(&value, _readPos, sizeof(D));
				_readPos += sizeof(D);
			};
			void get(std::string &value);
			void get(BString &value);
			void get(void *data, const TSize size);

			void reserve(const TSize newReservedSize);
			const TSize writtenSize() const
			{
				return _writePos - _begin;
			}
			const TSize readPos() const
			{
				return _readPos - _begin;
			}
			const bool isEnded() const
			{
				if (_readPos >= _writePos)
					return true;
				else
					return false;
			}
			const TSize reserved() const
			{
				return (_end - _begin);
			}
			void rewind()
			{
				_readPos = _begin;
			}
			void seekReadPos(const TSize seek);
			void truncate(const TSize seek);
			void clear()
			{
				_readPos = _begin;
				_writePos = _begin;
			}
			const bool empty() const
			{
				return (_writePos == _begin);
			}
			const TDataPtr begin() const
			{
				return _begin;
			}
			const TDataPtr readPtr() const
			{
				return _readPos;
			}
			
			TDataPtr release() noexcept;
			
			TDataPtr reserveBuffer(const TSize size);
			TDataPtr mapBuffer(const TSize size);
			void skip(const TSize size);
			TSize addSpace(const TSize size);
			
		private:
			void _fit(const TSize size);
			TDataPtr _begin;
			TDataPtr _end;
			TDataPtr _readPos;
			TDataPtr _writePos;
		};
	};
};

#endif	// __FL_BUFFER_HPP
