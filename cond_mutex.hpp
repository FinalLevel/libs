#pragma once
#ifndef __FL_COND_MUTEX_HPP
#define	__FL_COND_MUTEX_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Condition mutexes wrapper class
///////////////////////////////////////////////////////////////////////////////

#include <pthread.h>

namespace fl {
	namespace threads {
		class CondMutex
		{
		public:
			CondMutex();
			~CondMutex();
			void waitSignal();
			void sendSignal();
			void broadcastSignalToAll();
		private:
			pthread_mutex_t _mutex;
			pthread_cond_t _cond;
		};
	};
};

#endif	// __FL_COND_MUTEX_HPP
