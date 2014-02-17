///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Thread class
///////////////////////////////////////////////////////////////////////////////

#include "thread.hpp"

using namespace fl::threads;

Thread::Thread()
{
	pthread_attr_init(&_attr);
}

Thread::~Thread()
{
	pthread_attr_destroy(&_attr);
}

void Thread::setStackSize(unsigned int stackSize)
{
	pthread_attr_setstacksize(&_attr, stackSize);
};

void Thread::setDetachedState()
{
	pthread_attr_setdetachstate(&_attr, PTHREAD_CREATE_DETACHED);
}

bool Thread::create()
{
	if (pthread_create (&_threadId, &_attr, _action, (void *) this) == 0)
		return true;
	else
		return false;
}

void *Thread::_action(void *arg)
{
	Thread *thread = static_cast<Thread*>(arg);
	thread->run();
	pthread_exit(NULL);
	return 0;
}

void Thread::waitMe()
{
	void *retVal;
	pthread_join(_threadId, &retVal);
}

void Thread::cancel()
{
	pthread_cancel(_threadId);
}
