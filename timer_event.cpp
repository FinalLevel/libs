///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Timer events manipulations class implementation
///////////////////////////////////////////////////////////////////////////////

#include <sys/timerfd.h>
#include <unistd.h>

#include "timer_event.hpp"
#include "exception.hpp"

using namespace fl::events;

TimerEvent::TimerEvent(const time_t fromSeconds, const long int fromNanoSeconds, 
	const time_t everySeconds, const long int everyNanoSeconds)
	:Event(timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK))
{
	_op = EPOLL_CTL_ADD;
	_events = E_INPUT | E_ERROR;
	
	if (_descr == -1)
		throw exceptions::Error("Cannot create TimerEvent");	
	itimerspec tm;
	tm.it_value.tv_sec = fromSeconds;
	tm.it_value.tv_nsec = fromNanoSeconds;
	tm.it_interval.tv_sec = everySeconds; // make call every second
	tm.it_interval.tv_nsec = everyNanoSeconds;
	if (timerfd_settime(_descr, 0, &tm, 0))
		throw exceptions::Error("Cannot call  timerfd_settime in TimerEvent");	
}

TimerEvent::~TimerEvent()
{
	close(_descr);
}

Event::ECallResult TimerEvent::_readTimer()
{
	uint64_t readBuf;
	read(_descr, &readBuf, sizeof(readBuf));
	return SKIP;
}