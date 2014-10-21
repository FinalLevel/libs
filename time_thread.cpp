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

#include <stdint.h>
#include <cstring>
#include "time_thread.hpp"
#include "exception.hpp"
#include "log.hpp"

using namespace fl::threads;

TimeThread::TimeThread(const uint32_t tickSecTime, const uint32_t tickNonoTime)
	: _tickSecTime(tickSecTime), _tickNonoTime(tickNonoTime), _nextDailyCall(0), _nextHourlyCall(0)
{
	static const uint32_t MAX_TICK_TIME = 60 * 60; // 1 hour
	if (_tickSecTime > MAX_TICK_TIME)
		throw exceptions::Error("Too big tick time");	
	
	static const uint32_t TIME_THREAD_STACK_SIZE = 100000;
	setStackSize(TIME_THREAD_STACK_SIZE);
	if (!create())
		throw std::exception();
};

void TimeThread::_calcNextDailyCall()
{
	tm nextTm;
	memcpy(&nextTm, &_curTime.timeStruct(), sizeof(tm));
	nextTm.tm_sec = 0;
	nextTm.tm_min = 0;
	nextTm.tm_hour = 0;
	nextTm.tm_mday++;
	_nextDailyCall = mktime(&nextTm);
}

void TimeThread::_calcNextHourlyCall()
{
	tm nextTm;
	memcpy(&nextTm, &_curTime.timeStruct(), sizeof(tm));
	nextTm.tm_sec = 0;
	nextTm.tm_min = 0;
	nextTm.tm_hour++;
	_nextHourlyCall = mktime(&nextTm);	
}

void TimeThread::addEveryTick(BaseTimeTask *task)
{
	AutoMutex autoSync(&_newTasksSync);
	_newTasks.insert(TNewTaskMap::value_type(task, &_everyTickTasks));
}

void TimeThread::addEveryHour(BaseTimeTask *task)
{
	AutoMutex autoSync(&_newTasksSync);
	_newTasks.insert(TNewTaskMap::value_type(task, &_hourlyTasks));
}

void TimeThread::addEveryDay(BaseTimeTask *task)
{
	AutoMutex autoSync(&_newTasksSync);
	_newTasks.insert(TNewTaskMap::value_type(task, &_dailyTasks));
}

inline void TimeThread::_reloadTasks()
{
	if (_newTasks.empty())
		return;
	AutoMutex autoSync(&_newTasksSync);
	for (auto task = _newTasks.begin(); task != _newTasks.end(); task++)
		task->second->push_back(task->first);
	_newTasks.clear();
}


void TimeThread::run()
{
	uint32_t alignMinutes = (_tickSecTime + (_tickNonoTime / 1000000000)) / 60;
	struct timespec tim;
	_curTime.update();
	time_t lastDailyCall = 0;
	_calcNextHourlyCall();
	_calcNextDailyCall();
	
	while (true)
	{
		_reloadTasks();
		
		_curTime.update();
		tim.tv_sec = _tickSecTime;
		tim.tv_nsec = _tickNonoTime;
		
		if (alignMinutes > 0) { // align to near minutes
			tim.tv_sec = (_tickSecTime - ((_curTime.min() % alignMinutes) * 60)) - _curTime.sec();
			tim.tv_nsec = 0;
		}
		nanosleep(&tim , NULL);
		
		time_t lastWakeUP = _curTime.unix();
		_curTime.update();
		if ((uint32_t)abs(lastWakeUP - _curTime.unix()) > _tickSecTime * 2) // time maybe changed, skip this tick
			continue;
		
		for (auto task = _everyTickTasks.begin(); task != _everyTickTasks.end(); task++) {
			if (!(*task)->call(_curTime))
				break;
		}

		if (_curTime.unix() >= _nextHourlyCall) {
			for (auto task = _hourlyTasks.begin(); task != _hourlyTasks.end(); task++) {
				if (!(*task)->call(_curTime))
					break;
			}
			_calcNextHourlyCall();
		}
		
		if (_curTime.unix() >= _nextDailyCall) {
			static const time_t MINIMUM_DAILY_RUN_PERIOD = 21 * 3600;
			time_t lastTimeCalled = (_curTime.unix() - MINIMUM_DAILY_RUN_PERIOD);
			if (lastDailyCall < lastTimeCalled) {
				for (auto task = _dailyTasks.begin(); task != _dailyTasks.end(); task++) {
					if (!(*task)->call(_curTime))
						break;
				}
			}	else
				log::Error::L("Daily task has been already run (%u > %u)\n", lastDailyCall, lastTimeCalled);
			
			_calcNextDailyCall();
		}
	}
}
