#pragma once
#ifndef __FL_BSTRING_HPP
#define	__FL_BSTRING_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Buffered string class
///////////////////////////////////////////////////////////////////////////////

#include <cstdarg>
#include <string>
#include <cstring>
#include "exception.hpp"

namespace fl {
	namespace strings {
		
		class BString
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
			
			typedef int TSize;
			typedef char *TDataPtr;
			static const TSize DEFAULT_RESERVED_SIZE = 0;
			
			BString(const TSize reserved = DEFAULT_RESERVED_SIZE);
			BString(const char *str);
			~BString();
			
			BString(const BString &) = delete;
			BString& operator=(const BString &) = delete;
			
			BString(BString &&moveFrom);
			BString& operator=(BString &&moveFrom);
			
			TSize sprintfAdd(const char *fmt, ...);
			TSize sprintfSet(const char *fmt, ...);
			
			BString &operator<<(const char *str);
			BString &operator<<(const int num);
			BString &operator<<(const unsigned int num);
			BString &operator<<(const char ch);
			BString &operator<<(const std::string &str);
			
			void add(const char *str, TSize len);
			
			const bool operator==(const char *compareWith) const;
			
			void clear()
			{
				_size = 0;
				if (_data)
					_data[0] = 0;
			}
			const TSize size() const
			{
				return _size;
			}
			const TSize reserved() const
			{
				return _reserved;
			}
			const TDataPtr c_str() const
			{
				return _data;
			}
			TDataPtr data()
			{
				return _data;
			}
			TDataPtr reserveBuffer(const TSize size);
			void trim(TSize size);
			void trimLast();
			void reserve(const TSize newReservedSize);
			TDataPtr release();
		protected:
			bool _sprintfAdd(const char *fmt, TSize &charsAdded, va_list args);
			bool _reserveForSprintf(const int sprintfRes, const TSize leftSpace);
			void _fit(const TSize size);
			TSize _size;
			TSize _reserved;
			TDataPtr _data;
		};
	};
};

#endif	// __FL_BSTRING_HPP
