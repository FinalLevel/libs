///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Worker thread classes implementation
///////////////////////////////////////////////////////////////////////////////

#include "worker_thread.hpp"
#include "log.hpp"

using namespace fl::threads;

WorkerThreadManager::WorkerThreadManager(const size_t countThreads, const size_t workerThreadStackSize)
{
	for (size_t t = 0; t < countThreads; t++) {
		_threads.emplace_back(new WorkerThread(this, workerThreadStackSize));
	}
	log::Info::L("%u WorkerThreadManager have been started\n", _threads.size());
}

void WorkerThreadManager::add(WorkerTaskInterface *task)
{
	_sync.lock();
	_tasks.emplace_back(task);
	_sync.unLock();
	
	_threadCond.sendSignal();
}

void WorkerThreadManager::stopAndWait()
{
	for (auto thread = _threads.begin(); thread != _threads.end(); thread++) {
		(*thread)->stop();
	}
	_threadCond.broadcastSignalToAll();
	for (auto thread = _threads.begin(); thread != _threads.end(); thread++) {
		(*thread)->waitMe();
	}	
}

void WorkerThreadManager::doTasks(WorkerThread *thread)
{
	TWorkerTaskInterfaceVector tasks;
	while (true) {
		if (thread->isStopped()) {
			return;
		}
		_sync.lock();
		std::swap(_tasks, tasks);
		_sync.unLock();
		if (tasks.empty()) {
			_threadCond.waitSignal();
		} else {
			for (auto &task : tasks) {
				task->doTask();
			}
			tasks.clear();
		}
	}
}


WorkerThread::WorkerThread(WorkerThreadManager *manager, const size_t workerThreadStackSize)
	: _manager(manager), _stopped(false)
{
	setStackSize(workerThreadStackSize);
	if (!create()) {
		log::Fatal::L("WorkerThread can't be created");
		throw std::exception();
	}
}

WorkerThread::~WorkerThread()
{
	
}

void WorkerThread::run()
{
	_manager->doTasks(this);
}