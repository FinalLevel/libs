#pragma once
#ifndef __FL_THREADS_WORKER_THREAD_HPP
#define	__FL_THREADS_WORKER_THREAD_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Worker thread classes
///////////////////////////////////////////////////////////////////////////////

#include <list>
#include <vector>
#include <memory>
#include "thread.hpp"
#include "mutex.hpp"
#include "cond_mutex.hpp"


namespace fl {
	namespace threads {

		class WorkerTaskInterface
		{
		public:
			virtual void doTask() = 0;
			virtual ~WorkerTaskInterface()
			{
			}
		};

		class WorkerThread : public Thread
		{
		public:
			WorkerThread(class WorkerThreadManager *manager, const size_t workerThreadStackSize);
			virtual ~WorkerThread();
			void stop()
			{
				_stopped = true;
			}
			const bool isStopped() const
			{
				return _stopped;
			}
		private:
			virtual void run();
			class WorkerThreadManager *_manager;
			bool _stopped;
		};

		class WorkerThreadManager
		{
		public:
			static const size_t USER_LOAD_THREAD_STACK_SIZE = 100000;
			WorkerThreadManager(const size_t countThreads, const size_t workerThreadStackSize = USER_LOAD_THREAD_STACK_SIZE);
			void add(WorkerTaskInterface *task);

			void doTasks(WorkerThread *thread);
			void stopAndWait();
		private:
			typedef std::list<WorkerTaskInterface*> TWorkerTaskInterfaceVector;
			TWorkerTaskInterfaceVector _tasks;

			Mutex _sync;

			CondMutex _threadCond;

			typedef std::unique_ptr<WorkerThread> TUserLoadThreadPtr;
			typedef std::vector<TUserLoadThreadPtr> TUserLoadThreadPtrVector;
			TUserLoadThreadPtrVector _threads;
		};
	};
};


#endif	// __FL_THREADS_WORKER_THREAD_HPP
