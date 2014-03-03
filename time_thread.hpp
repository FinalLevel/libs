#pragma once
#ifndef __FL_TIME_THREAD_HPP
#define	__FL_TIME_THREAD_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: TimeThread class implementing a thread which calls specified functions in the specified moment of time 
// (like a crontab file in Unix like systems.)
///////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <map>
#include "time.hpp"
#include "mutex.hpp"
#include "thread.hpp"

namespace fl {
	namespace threads {
		using fl::chrono::ETime;
		
		class BaseTimeTask
		{
		public:
			virtual bool call(ETime &curTime) = 0; 
		};
		
		template<class T>
		class TimeTask : public BaseTimeTask
		{
		private:
			typedef bool (T::*TMethod)(ETime &curTime);
			T *_cl;
			TMethod _method;
		public:
			TimeTask(T *cl, TMethod method)
				: _cl(cl), _method(method)
			{
			}
			virtual bool call(ETime &curTime)
			{
				return (_cl->*_method)(curTime);
			}
		};
		
		class TimeThread : public Thread
		{
		public:
			TimeThread(const uint32_t tickSecTime, const uint32_t tickNonoTime = 0);
			virtual ~TimeThread() {};
			void addEveryTick(BaseTimeTask *task);
			void addEveryHour(BaseTimeTask *task);
			void addEveryDay(BaseTimeTask *task);
		private:
			virtual void run();
			void _calcNextDailyCall();
			void _calcNextHourlyCall();
			void _reloadTasks();
			
			uint32_t _tickSecTime;
			uint32_t _tickNonoTime;
			ETime _curTime;
			typedef std::vector<BaseTimeTask*> TTimeTaskVector;
			TTimeTaskVector _everyTickTasks;
			TTimeTaskVector _hourlyTasks;
			TTimeTaskVector _dailyTasks;
			typedef std::map<BaseTimeTask*, TTimeTaskVector*> TNewTaskMap;
			TNewTaskMap _newTasks;
			Mutex _newTasksSync;
			
			time_t _nextDailyCall;
			time_t _nextHourlyCall;
		};
	};
};

#endif	// __FL_TIME_THREAD_HPP
