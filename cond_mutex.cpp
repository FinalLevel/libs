///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Condition mutexes wrapper class
///////////////////////////////////////////////////////////////////////////////

#include "cond_mutex.hpp"

using namespace fl::threads;

CondMutex::CondMutex()
{
	 pthread_mutex_init(&_mutex, NULL);
	 pthread_cond_init(&_cond, NULL);
}

CondMutex::~CondMutex()
{
	pthread_mutex_destroy(&_mutex);
	pthread_cond_destroy(&_cond);
}

void CondMutex::waitSignal()
{
	pthread_mutex_lock(&_mutex);
	pthread_cond_wait(&_cond, &_mutex);
	pthread_mutex_unlock(&_mutex);		
}

void CondMutex::sendSignal()
{
	pthread_cond_signal(&_cond);
}

void CondMutex::broadcastSignalToAll()
{
	pthread_cond_broadcast(&_cond);
}			
