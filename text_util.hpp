#pragma once
#ifndef __FL_TEXT_UTIL_HPP
#define	__FL_TEXT_UTIL_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Different text processing utility functions 
///////////////////////////////////////////////////////////////////////////////

#include "bstring.hpp"

namespace fl {
	namespace utils {
		void quotedPrintableDecode(fl::strings::BString &result, const char *input, const size_t size, 
			const char delim = '=');
	};
};

#endif	// __FL_TEXT_UTIL_HPP
