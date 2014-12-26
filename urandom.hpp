#pragma once
#ifndef __FL_URANDOM_HPP
#define	__FL_URANDOM_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: /dev/urandom wrapper class
///////////////////////////////////////////////////////////////////////////////

#include "file.hpp"


namespace fl {
	namespace utils {
		class URandom
		{
		public:
			URandom();
			bool getBlock(void *data, const size_t size);
		private:
			fl::fs::File _fd;
		};
	};
};

#endif	// __FL_URANDOM_HPP
