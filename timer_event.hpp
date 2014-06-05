#pragma once
#ifndef __FL_TIMER_EVENT_HPP
#define	__FL_TIMER_EVENT_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Timer events manipulations class
///////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include "event_queue.hpp"

namespace fl {
	namespace events {
		class TimerEventInterface
		{
		public:
			virtual void timerCall(class TimerEvent *te) = 0;
		};
		
		class TimerEvent : public Event
		{
		public:
			TimerEvent();
			bool setTimer(const time_t fromSeconds, const long int fromNanoSeconds, 
				const time_t everySeconds, const long int everyNanoSeconds, TimerEventInterface *interface = NULL);
			void stop();
	
			virtual ~TimerEvent();
			virtual const ECallResult call(const TEvents events);
		protected:
			Event::ECallResult _readTimer();
			TimerEventInterface *_interface;
		};
	};
};

#endif	// __FL_TIMER_EVENT_HPP
