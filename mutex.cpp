///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Mutex primitive wrapper class
///////////////////////////////////////////////////////////////////////////////

#include "mutex.hpp"

using namespace fl::threads;

Mutex::Mutex()
{
	pthread_mutex_init(&_mutex, NULL);
}

Mutex::~Mutex()
{
	pthread_mutex_destroy(&_mutex);
}

void Mutex::lock()
{
	  pthread_mutex_lock(&_mutex);
}

bool Mutex::tryLock()
{
	if (pthread_mutex_trylock(&_mutex)) // mute busy
		return false;
	return true;
}

void Mutex::unLock()
{
	  pthread_mutex_unlock(&_mutex);
}


