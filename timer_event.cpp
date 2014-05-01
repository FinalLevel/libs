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
#include "socket.hpp"
#include "log.hpp"

using namespace fl::events;

TimerEvent::TimerEvent()
	: Event(INVALID_EVENT), _interface(NULL)
{
}

bool TimerEvent::setTimer(const time_t fromSeconds, const long int fromNanoSeconds, 
	const time_t everySeconds, const long int everyNanoSeconds, TimerEventInterface *interface)
{
	if (_descr != INVALID_EVENT)
		close(_descr);
	_descr = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
	if (_descr == INVALID_EVENT)
		return false;
	_op = EPOLL_CTL_ADD;
	_events = E_INPUT | E_ERROR;
	
	itimerspec tm;
	tm.it_value.tv_sec = fromSeconds;
	tm.it_value.tv_nsec = fromNanoSeconds;
	tm.it_interval.tv_sec = everySeconds; // make call every second
	tm.it_interval.tv_nsec = everyNanoSeconds;
	if (timerfd_settime(_descr, 0, &tm, 0)) {
		log::Error::L("Cannot call  timerfd_settime in TimerEvent");
		return false;
	}
	_interface = interface;
	return true;
}

void TimerEvent::stop()
{
	if (_descr != INVALID_EVENT) {
		close(_descr);
		_descr = INVALID_EVENT;
	}
	_interface = NULL;
}

TimerEvent::~TimerEvent()
{
	if (_descr != INVALID_EVENT)
		close(_descr);
}

Event::ECallResult TimerEvent::_readTimer()
{
	uint64_t readBuf;
	read(_descr, &readBuf, sizeof(readBuf));
	return SKIP;
}

const TimerEvent::ECallResult TimerEvent::call(const TEvents events)
{
	auto res = _readTimer();
	if (_interface)
		_interface->timerCall(this);
	return res;
}
