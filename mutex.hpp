#pragma once
#ifndef __FL_MUTEX_HPP__
#define	__FL_MUTEX_HPP__

///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Mutex primitive wrapper class
///////////////////////////////////////////////////////////////////////////////

#include <pthread.h>

namespace fl {
	namespace threads {
		class Mutex
		{
		public:
			Mutex();
			~Mutex();
			void lock();
			bool tryLock();
			void unLock();
		private:
			pthread_mutex_t _mutex;
		};

		class AutoMutex
		{
		public:
			AutoMutex()
				: _sync(NULL)
			{
			}
			AutoMutex(Mutex *sync)
				: _sync(sync)
			{
				_sync->lock();
			}
			~AutoMutex()
			{
				if (_sync) {
					_sync->unLock();
				}
			}
			void lock(Mutex *sync)
			{
				_sync = sync;
				_sync->lock();
			}
			void unLock()
			{
				if (_sync) {
					_sync->unLock();
					_sync = NULL;
				}
			}
			bool tryLock(Mutex *sync)
			{
				if (sync->tryLock()) {
					_sync = sync;
					return true;
				}
				else
					return false;
			}			
			AutoMutex(const AutoMutex &autoMutex) = delete;
		private:
			Mutex *_sync;
		};
	};
};

#endif	// __FL_MUTEX_HPP__
