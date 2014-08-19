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
			Mutex(const Mutex&) = delete;
			Mutex &operator=(const Mutex&) = delete;
			
			Mutex(const Mutex&&) = delete;
			Mutex &operator=(Mutex&&) = delete;
			
			void lock();
			bool tryLock();
			void unLock();
		private:
			pthread_mutex_t _mutex;
		};

		class WeekAutoMutex
		{
		public:
			WeekAutoMutex()
				: _sync(NULL), _locked(false)
			{
			}
			WeekAutoMutex(Mutex *sync)
				: _sync(sync), _locked(true)
			{
					_sync->lock();
			}
			~WeekAutoMutex()
			{
				if (_locked) {
					_sync->unLock();
				}
			}
			void resetAndLock(Mutex *sync)
			{
				unLock();
				_sync = sync;
				lock();
			}
			void lock()
			{
				if (_locked)
					return;
				_sync->lock();
				_locked = true;
			}
			void unLock()
			{
				if (!_locked)
					return;
				_sync->unLock();
				_locked = false;
			}
			WeekAutoMutex(const WeekAutoMutex &) = delete;
			WeekAutoMutex& operator=(const WeekAutoMutex &) = delete;
		private:
			Mutex *_sync;
			bool _locked;
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
			
			AutoMutex(const AutoMutex &autoMutex) = delete;
			AutoMutex& operator=(const AutoMutex &) = delete;
			
			AutoMutex(AutoMutex &&autoSync)
				: _sync(autoSync._sync)
			{
				autoSync._sync = NULL;
			}
			
			AutoMutex &operator=(AutoMutex &&autoSync);
			
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
		private:
			Mutex *_sync;
		};
	};
};

#endif	// __FL_MUTEX_HPP__
