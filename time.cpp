///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Time functions wrapper class
///////////////////////////////////////////////////////////////////////////////

#include "time.hpp"

using namespace fl::chrono;

Time::Time()
	: _unix(time(NULL))
{
	
}

void Time::update()
{
	_unix = time(NULL);
}
