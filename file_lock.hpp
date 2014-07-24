#pragma once
#ifndef __FL_FILE_LOCK_HPP
#define	__FL_FILE_LOCK_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Basic file locking class
///////////////////////////////////////////////////////////////////////////////

#include <string>
#include "bstring.hpp"

namespace fl {
	namespace fs {
		class FileLock
		{
		public:
			class Error {};
			FileLock(const std::string &fileName, const size_t maxWaitTime);
			~FileLock();
			bool checkKey();
		private:
			void _writeKey();
			fl::strings::BString _lockFileName;
			uint64_t _lockKey;
		};
	};
};

#endif	// __FL_FILE_LOCK_HPP
