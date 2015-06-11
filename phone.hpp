#pragma once
#ifndef __FL_PHONE_HPP
#define	__FL_PHONE_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Phone numbers manipulation functions
///////////////////////////////////////////////////////////////////////////////

#include <string>

namespace fl {
	namespace utils {
		static const size_t MIN_INT_PHONE_LENGTH = 1 /* + */ + 1 /* min country code length */ + 5 /* phone number */; 
		static const size_t MAX_INT_PHONE_LENGTH = 1 /* + */ + 3 /* max country code length */ + 3 /* code */ 
			+ 7 /* phone number */;
		uint64_t formInternationalPhone(const std::string &phone, const uint32_t countryPrefix);
	};
};

#endif	// __FL_PHONE_HPP
