///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Classes for events management 
///////////////////////////////////////////////////////////////////////////////

#include <errno.h>
#include <unistd.h>
#include <cstring>
#include "event_queue.hpp"
#include "log.hpp"

using namespace fl::events;

EPoll::EPoll(const int queueLength)
	: _queueLength(queueLength), _activeEventsCount(0)
{
	if ((_eventFD = epoll_create(_queueLength)) == -1) 
	{
		throw EPollErorr("Cannot create epoll queue");
	}
	_events = new epoll_event[_queueLength];
};

EPoll::~EPoll()
{
	close(_eventFD);
	delete [] _events;
}

bool EPoll::ctrl(Event *event, const TEventDescriptor descr, const int op, const TEvents events)
{
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.data.ptr = event;
	ev.events = events;
	
	if (epoll_ctl(_eventFD, op, descr, &ev) == -1)
		return false;
	
	return true;
}

bool EPoll::ctrl(Event *event)
{
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.data.ptr = event;
	ev.events = event->events();
	
	if (epoll_ctl(_eventFD, event->op(), event->descr(), &ev) == -1)
	{
		log::Error::L("epoll_ctl erorr %d - %s - %d - %d (%d)\n", errno, strerror(errno), _eventFD, event->descr(), event->op());
		return false;
	}

	event->setOp(EPOLL_CTL_MOD);
	
	return true;
}

bool EPoll::dispatch(const int timeout)
{
	_activeEventsCount = 0;
	int res = epoll_wait(_eventFD, _events, _queueLength, timeout);

	if (res == -1) 
	{
		if (errno == EINTR || errno == EAGAIN)
			return true;
		return false;
	} 
	_activeEventsCount = res;
	return true;
}

bool EPoll::callActive(TEventVector &changedEvents, TEventVector &endedEvents)
{
	for (int i = 0; i < _activeEventsCount; i++) 
	{
		Event *ev = static_cast<Event *>(_events[i].data.ptr);
		Event::ECallResult res = ev->call(_events[i].events);
		switch (res)
		{
			case Event::CHANGE:
				changedEvents.push_back(ev);
			break;
			case Event::FINISHED:
				endedEvents.push_back(ev);
			break;
			case Event::SKIP:
			break;
		}
	}
	return _activeEventsCount > 0;
}

Event::Event(const TEventDescriptor descr)
	: _descr(descr), _op(EPOLL_CTL_ADD), _events(0)
{
	
}
