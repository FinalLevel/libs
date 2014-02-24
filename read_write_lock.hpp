#pragma once
#ifndef __FL_READ_WRITE_LOCK_HPP
#define	__FL_READ_WRITE_LOCK_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: RW lock wrapper classes
///////////////////////////////////////////////////////////////////////////////

#include <pthread.h>

namespace fl {
	namespace threads {
		
		class ReadWriteLock 
		{
		public:
			ReadWriteLock();
			~ReadWriteLock();
			void readLock();
			void writeLock();
			void unLock();			
		private:
			pthread_rwlock_t _lock;
		};
		
		class AutoReadWriteLockRead
		{
		public:
			AutoReadWriteLockRead(ReadWriteLock *lock)
				: _lock(lock)
			{
				_lock->readLock();
			}
			~AutoReadWriteLockRead()
			{
				if (_lock)
					_lock->unLock();
			}
			void unLock()
			{
				if (_lock)
				{
					_lock->unLock();
					_lock = NULL;
				}
			}
			void lock(ReadWriteLock *lock)
			{
				_lock = lock;
				_lock->readLock();
			}
		private:
			ReadWriteLock *_lock;
		};

		class AutoReadWriteLockWrite
		{
		public:
			AutoReadWriteLockWrite(ReadWriteLock *lock)
				: _lock(lock)
			{
				_lock->writeLock();
			}
			~AutoReadWriteLockWrite()
			{
				if (_lock)
					_lock->unLock();
			}
			void unLock()
			{
				if (_lock)
				{
					_lock->unLock();
					_lock = NULL;
				}
			}
			void lock(ReadWriteLock *lock)
			{
				_lock = lock;
				_lock->writeLock();
			}
		private:
			ReadWriteLock *_lock;
		};
		
	};
};

#endif	// __FL_READ_WRITE_LOCK_HPP
