#pragma once
#ifndef __FL_EVENT_THREAD_HPP__
#define	__FL_EVENT_THREAD_HPP__

///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: An event system worker class
///////////////////////////////////////////////////////////////////////////////

#include <sys/epoll.h>
#include <cstdint>
#include <list>
#include <vector>

#include "event_queue.hpp"
#include "thread.hpp"
#include "mutex.hpp"
#include "time.hpp"

namespace fl {
	namespace events {
		
		typedef std::list<class WorkEvent*> TWorkEventList;
		
		class EPollWorkerThread : public fl::threads::Thread
		{
		public:
			EPollWorkerThread(
				const uint32_t queueLength, 
				class ThreadSpecificData* threadSpecificData, 
				const uint32_t stackSize
			);
			bool ctrl(class WorkEvent *ue)
			{
				return _poll.ctrl(ue);
			}
			bool addConnection(class WorkEvent* ev, class Socket *acceptSocket);
			static fl::chrono::Time curTime;
		private:
			class UpdateTimeEvent : public Event
			{
			public:
				UpdateTimeEvent();
				virtual ~UpdateTimeEvent();
				virtual const ECallResult call(const TEvents events);
			};
			static Event *_updateTimeEvent;
			virtual void run();
			EPoll _poll;
			class ThreadSpecificData *_threadSpecificData;
			
			void _addEvent(class WorkEvent *ev);
			TWorkEventList _events;
			fl::threads::Mutex _eventsSync;
		};
		
		
		class EPollWorkerGroup
		{
		public:
			static const uint32_t EPOLL_WORKER_STACK_SIZE = 100 * 1024;
			EPollWorkerGroup(
				class ThreadSpecificDataFactory *factory,
				const uint32_t maxWorkers, 
				const uint32_t queueLength, 
				const uint32_t stackSize = EPOLL_WORKER_STACK_SIZE
			);
			bool addConnection(class WorkEvent* ev, class Socket *acceptSocket);
		private:
			typedef std::vector<EPollWorkerThread*> TWorkerThreadVector;
			TWorkerThreadVector _threads;
		};

		class WorkEvent : public Event
		{
		public:
			WorkEvent(const TEventDescriptor descr, const time_t timeOutTime);
			void setListPosition(const TWorkEventList::iterator listPosition)
			{
				_listPosition = listPosition;
			}
			const TWorkEventList::iterator listPosition() const
			{
				return _listPosition;
			}
			typedef uint16_t TStatus;
			static const TStatus ST_ACTIVE = 0x1;
			static const TStatus ST_FINISHED = 0x2;
			
			virtual const bool isFinished() const
			{
				return true;
			}
			
			const bool isActive() const
			{
				return _status & ST_ACTIVE;
			}
			void clearActive()
			{
				_status &= (~ST_ACTIVE);
			}
			const time_t timeOutTime() const
			{
				return _timeOutTime;
			}
			void setThread(EPollWorkerThread *thread)
			{
				_thread = thread;
			}
		private:
			EPollWorkerThread *_thread;
			TWorkEventList::iterator _listPosition;
			time_t _timeOutTime;
			TStatus _status;
		};

		class ThreadSpecificData
		{
		public:
		};
		
		class ThreadSpecificDataFactory 
		{
		public:
			virtual ThreadSpecificData *create() 
			{ 
				return NULL; 
			};
			virtual ~ThreadSpecificDataFactory() = default;
		};

		class WorkEventFactory 
		{
		public:
			virtual WorkEvent *create(const TEventDescriptor descr, const time_t timeOutTimet, class Socket *acceptSocket) = 0;
			virtual ~WorkEventFactory() = default;
		};
		
	};
};

#endif	// __FL_EVENT_THREAD_HPP__
