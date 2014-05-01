#pragma once
#ifndef __FL_EVENT_QUEUE_HPP__
#define	__FL_EVENT_QUEUE_HPP__

///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Classes for events management 
///////////////////////////////////////////////////////////////////////////////

#include <sys/epoll.h>
#include <cstdint>
#include <vector>

#include "exception.hpp"

namespace fl {
	namespace events {
		
		typedef int TEventDescriptor;
		typedef uint32_t TEvents;
		const TEventDescriptor INVALID_EVENT = -1;
		
		const TEvents E_OUTPUT = EPOLLOUT;
		const TEvents E_INPUT	 = EPOLLIN;
		const TEvents E_ERROR	 = EPOLLERR;
		const TEvents E_HUP		 = EPOLLHUP;

		class EPoll
		{
		public:
			class EPollErorr : public exceptions::Error 
			{
			public:
				EPollErorr(const char *what)
					: Error(what)
				{
				}
			};
			
			explicit EPoll(const int queueLength);
			~EPoll();
			bool ctrl(class Event *event);
			bool ctrl(class Event *event, const TEventDescriptor descr, const int op, const TEvents events);
			
			bool dispatch(const int timeout);
			
			typedef std::vector<class Event*> TEventVector;
			bool callActive(TEventVector &changedEvents, TEventVector &endedEvents);
			const int queueLength() const
			{
				return _queueLength;
			}
		private:
			int _eventFD;
			int _queueLength;
			struct epoll_event *_events;
			int _activeEventsCount;
		};
		
		class Event
		{
		public:
			explicit Event(const TEventDescriptor descr);
			virtual ~Event() {};
			
			void clearEvent(int mask)
			{
				_events &= ~(mask);
			}	
			
			void addEvent(int mask)
			{
				_events |= mask;
			}
			
			const TEvents events() const
			{
				return _events;
			}
			const int op() const
			{
				return _op;
			}
			const TEventDescriptor descr()
			{
				return _descr;
			}
			void setOp(const int op)
			{
				_op = op;
			}
			enum ECallResult
			{
				CHANGE,
				FINISHED,
				SKIP,
			};
			virtual const ECallResult call(const TEvents events) = 0;
			void setWaitRead();
			void setWaitSend();			
		protected:
			TEventDescriptor _descr;
			int _op;
			TEvents _events;
		};
	};
};

#endif	// __FL_EVENT_QUEUE_HPP__
