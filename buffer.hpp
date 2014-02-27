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

namespace fl {
	namespace utils {
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
			typedef size_t TSize;
			typedef u_int8_t *TDataPtr;
			Buffer(const TSize reserved = 0);
			~Buffer();
			Buffer(const Buffer &) = delete;
			
			template<class D> void add(const D &value)
			{
				_fit(sizeof(D));
				memcpy(_writePos, &value, sizeof(D));
				_writePos += sizeof(D);
			};
			void add(const std::string &value);
			void add(const void *data, const TSize size);
			
			template<class D> void get(D &value)
			{
				if ((_readPos + sizeof(D)) > _writePos)
					throw Error("Read out of range");
				memcpy(&value, _readPos, sizeof(D));
				_readPos += sizeof(D);
			};
			void get(std::string &value);
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
			TDataPtr reserveBuffer(const TSize size);
			TDataPtr mapBuffer(const TSize size);
			void skip(const TSize size);
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
