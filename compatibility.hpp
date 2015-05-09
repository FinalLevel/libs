#pragma once
#ifndef __FL_COMPATIBILITY_HPP
#define	__FL_COMPATIBILITY_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Several defines to compile code with c++11 features within c++0x compiler
///////////////////////////////////////////////////////////////////////////////

#include "config.h"

#ifndef HAVE_CXX11
	#define override
	#define noexcept
#endif


#endif	// __FL_COMPATIBILITY_HPP
