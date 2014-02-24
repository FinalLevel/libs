///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: RW lock wrapper classes
///////////////////////////////////////////////////////////////////////////////

#include "read_write_lock.hpp"

using namespace fl::threads;

ReadWriteLock::ReadWriteLock()
{
	pthread_rwlock_init(&_lock, NULL);
}

ReadWriteLock::~ReadWriteLock()
{
	pthread_rwlock_destroy(&_lock);
}

void ReadWriteLock::readLock()
{
		pthread_rwlock_rdlock(&_lock);
}

void ReadWriteLock::writeLock()
{
		pthread_rwlock_wrlock(&_lock);
}

void ReadWriteLock::unLock()
{
		pthread_rwlock_unlock(&_lock);
}
