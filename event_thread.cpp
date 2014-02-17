///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: An event system worker class
///////////////////////////////////////////////////////////////////////////////

#include <sys/timerfd.h>
#include <unistd.h>

#include "event_thread.hpp"
#include "exception.hpp"


using namespace fl::events;
using namespace fl::threads;
using fl::chrono::Time;

Time EPollWorkerThread::curTime;
Event *EPollWorkerThread::_updateTimeEvent = NULL;

EPollWorkerThread::EPollWorkerThread(
	const uint32_t queueLength, 
	class ThreadSpecificData* threadSpecificData, 
	const uint32_t stackSize
)
	: _poll(queueLength), _threadSpecificData(threadSpecificData)
{
	if (!_updateTimeEvent) { // start global timer for timer update
		_updateTimeEvent = new UpdateTimeEvent();
		_poll.ctrl(_updateTimeEvent);
	}
	
	setDetachedState();
	setStackSize(stackSize);
	if (!create())
		throw exceptions::Error("Cannot create EPollWorkerThread thread");	
}

bool EPollWorkerThread::addConnection(WorkEvent* ev, class Socket *acceptSocket)
{
	AutoMutex autoSync(&_eventsSync);
	if (!ctrl(ev))
		return false;
	
	ev->setThread(this);
	_addEvent(ev);
	return true;
}

inline void EPollWorkerThread::_addEvent(WorkEvent *ev)
{
	for (auto eventIter = _events.rbegin(); eventIter != _events.rend(); eventIter++)
	{
		if ((*eventIter)->timeOutTime() < ev->timeOutTime())
		{
			ev->setListPosition(_events.insert(eventIter.base(), ev));
			break;
		}
	}
	ev->setListPosition(_events.insert(_events.rend().base(), ev));
}

void EPollWorkerThread::run()
{
	EPoll::TEventVector changedEvents;
	EPoll::TEventVector endedEvents;
	time_t lastCheckTime = 0;
	while (1)
	{
		static const int EVENT_WAIT_TIME = 1; // wait 1 second
		_poll.dispatch(EVENT_WAIT_TIME * 1000);
		_eventsSync.lock();
		if (_poll.callActive(changedEvents, endedEvents)) {
			
			for (auto eventIter = changedEvents.begin(); eventIter != changedEvents.end(); eventIter++) {
				WorkEvent *event = static_cast<WorkEvent*>(*eventIter);
				if (!event->isActive()) // prevent double add events with timer
					continue;
				_addEvent(event);
				event->clearActive();
			}
			
			for (auto eventIter = endedEvents.begin(); eventIter != endedEvents.end(); eventIter++) {
				WorkEvent *event = static_cast<WorkEvent*>(*eventIter);
				if (!event->isActive())
					*eventIter = NULL;
				
				_events.erase(event->listPosition());
				event->clearActive();
			}
			for (auto eventIter = endedEvents.begin(); eventIter != endedEvents.end(); eventIter++)
				delete (*eventIter);
		}
		if (lastCheckTime != curTime.unix()) {
			lastCheckTime = curTime.unix();
			
			for (auto eventIter = _events.begin(); eventIter != _events.end(); eventIter++)  {
				if ((*eventIter)->timeOutTime() > lastCheckTime) // only new events left
					break;
				
				if ((*eventIter)->isFinished()) {
					WorkEvent *event = (*eventIter);
					eventIter = _events.erase(eventIter);
					delete event;
				}
			}
		}
		_eventsSync.unLock();
	}
}

EPollWorkerGroup::EPollWorkerGroup(
	ThreadSpecificDataFactory *factory,
	const uint32_t maxWorkers, 
	const uint32_t queueLength, 
	const uint32_t stackSize
)
{
	for (uint32_t i = 0; i < maxWorkers; i++)
	{
		_threads.push_back(new EPollWorkerThread(queueLength, factory->create(), stackSize));
	}
}

bool EPollWorkerGroup::addConnection(class WorkEvent* ev, class Socket *acceptSocket)
{
	static int curRnd = 0;
	curRnd++;
	if (_threads[_threads.size() % curRnd]->addConnection(ev, acceptSocket))
		return true;
	for (auto thread = _threads.begin(); thread != _threads.begin(); thread++) {
		if ((*thread)->addConnection(ev, acceptSocket))
			return true;
	}
	return false;
}

WorkEvent::WorkEvent(const TEventDescriptor descr, const time_t timeOutTime)
	: Event(descr), _thread(NULL), _timeOutTime(timeOutTime), _status(0)
{
	
}

EPollWorkerThread::UpdateTimeEvent::UpdateTimeEvent()
	: Event(timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK))
{
	if (_descr == -1)
		throw exceptions::Error("Cannot create UpdateTimeEvent");	
	itimerspec tm;
	tm.it_value.tv_sec = 1;
	tm.it_value.tv_nsec = 0;
	tm.it_interval.tv_sec = 1; // make call every second
	tm.it_interval.tv_nsec = 0;
	if (timerfd_settime(_descr, 0, &tm, 0))
		throw exceptions::Error("Cannot call  timerfd_settime in UpdateTimeEvent");	
}

EPollWorkerThread::UpdateTimeEvent::~UpdateTimeEvent()
{
	close(_descr);
}

const Event::ECallResult EPollWorkerThread::UpdateTimeEvent::call(const TEvents events)
{
	EPollWorkerThread::curTime.update();
	return Event::SKIP;
}
