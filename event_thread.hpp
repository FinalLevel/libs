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
#include "timer_event.hpp"
#include "thread.hpp"
#include "mutex.hpp"
#include "time.hpp"
#include "socket.hpp"

namespace fl {
	namespace events {
		using namespace fl::network;
		
		typedef std::list<class WorkEvent*> TWorkEventList;
		
		class WorkEvent : public Event
		{
		public:
			WorkEvent(const TEventDescriptor descr, const time_t timeOutTime);
			virtual ~WorkEvent() 
			{
			};
			
			void setListPosition(const TWorkEventList::iterator listPosition)
			{
				_listPosition = listPosition;
			}
			const TWorkEventList::iterator listPosition() const
			{
				return _listPosition;
			}
			virtual bool isFinished()
			{
				return true;
			}
			
			const time_t timeOutTime() const
			{
				return _timeOutTime;
			}
			void setThread(class EPollWorkerThread *thread)
			{
				_thread = thread;
			}
			class EPollWorkerThread *thread()
			{
				return _thread;
			}
		protected:
			class EPollWorkerThread *_thread;
			time_t _timeOutTime;
		private:
			TWorkEventList::iterator _listPosition;
		};
		
		class EPollWorkerThread : public fl::threads::Thread
		{
		public:
			EPollWorkerThread(
				const uint32_t queueLength, 
				class ThreadSpecificData* threadSpecificData, 
				const uint32_t stackSize
			);
			virtual ~EPollWorkerThread();
			bool ctrl(class Event *ue)
			{
				return _poll.ctrl(ue);
			}
			bool addConnection(class WorkEvent* ev, Socket *acceptSocket);
			class ThreadSpecificData *threadSpecificData()
			{
				return _threadSpecificData;
			}
			void addToDeletedNL(class Event *ev);
			bool unAttachNL(class WorkEvent* ev);
			bool addEvent(class WorkEvent *ev);
		private:
			virtual void run();
			EPoll _poll;
			class ThreadSpecificData *_threadSpecificData;
			
			void _addEvent(class WorkEvent *ev);
			TWorkEventList _events;
			fl::threads::Mutex _eventsSync;
			typedef std::vector<class Event *> TEventList;
			TEventList _deletedEvents;
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
			~EPollWorkerGroup();
			bool addConnection(class WorkEvent* ev, Socket *acceptSocket);
			
			static fl::chrono::Time curTime; // time value updated by UpdateTimeEvent
			class UpdateTimeEvent : public TimerEvent
			{
			public:
				UpdateTimeEvent();
				virtual ~UpdateTimeEvent() {};
				virtual const ECallResult call(const TEvents events);
			};
			void waitThreads();
			void cancelThreads();
			EPollWorkerThread *getThread(size_t number);
		private:
			static UpdateTimeEvent *_updateTimeEvent;
			typedef std::vector<EPollWorkerThread*> TWorkerThreadVector;
			TWorkerThreadVector _threads;
		};


		class ThreadSpecificData
		{
		public:
			virtual ~ThreadSpecificData() {};
		};
		
		class ThreadSpecificDataFactory 
		{
		public:
			virtual ThreadSpecificData *create() 
			{ 
				return NULL; 
			};
			virtual ~ThreadSpecificDataFactory() {};
		};

		class WorkEventFactory 
		{
		public:
			virtual WorkEvent *create(const TEventDescriptor descr, const TIPv4 ip, const time_t timeOutTimet, 
				Socket *acceptSocket) = 0;
			virtual ~WorkEventFactory() {};
		};
		
	};
};

#endif	// __FL_EVENT_THREAD_HPP__
