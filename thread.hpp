#pragma once
#ifndef __FL_THREAD_HPP__
#define	__FL_THREAD_HPP__

///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Thread class
///////////////////////////////////////////////////////////////////////////////

#include <pthread.h>

namespace fl {
	namespace threads {
		
		class Thread
		{
		public:
			Thread();
			Thread(const Thread&) = delete;
			virtual ~Thread();
			bool create();
			void setStackSize(unsigned int steckSize);
			void setDetachedState();
			void waitMe();
			void cancel();
		protected:
			static void* _action(void *);
			virtual void run() = 0;
			
			pthread_t _threadId;
			pthread_attr_t _attr;
		};
		
	};
};

#endif	// __FL_THREAD_HPP__
