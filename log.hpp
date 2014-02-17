#pragma once
#ifndef __FL_LOG_HPP__
#define __FL_LOG_HPP__

///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: A logging system facilities
///////////////////////////////////////////////////////////////////////////////

#ifndef FL_LOG_LEVEL // set default log level to WARNING
#define FL_LOG_LEVEL 2
#endif

#include <cstdarg>
#include <ctime>
#include <cstdio>
#include <vector>

namespace fl {
	namespace log {
		
		namespace ELogLevel {
			enum ELogLevel
			{
				FATAL = 1,
				ERROR,
				WARNING,
				INFO,
				MAX_LOG_LEVEL,
			};
		};
		
		class Target
		{
		public:
			virtual void log(
				const int level, 
				const char *tag, 
				const time_t curTime, 
				struct tm *ct, 
				const char *fmt, 
				va_list args
			) = 0;
		protected:
			struct ProcessInfo
			{
				ProcessInfo();
				pid_t pid;
			};
			static ProcessInfo _process;
		};
		
		class ScreenTarget : public Target
		{
		public:
			virtual void log(
				const int level, 
				const char *tag, 
				const time_t curTime, 
				struct tm *ct, 
				const char *fmt, 
				va_list args
			);
		};
		
		class FileTarget : public Target
		{
		public:
			FileTarget(const char *fileName);
			virtual ~FileTarget();
			virtual void log(
				const int level, 
				const char *tag, 
				const time_t curTime, 
				struct tm *ct, 
				const char *fmt, 
				va_list args
			);
		private:
			FILE *_fd;
		};
		
		class LogSystem
		{
		public:
			static bool log(
				const size_t target, 
				const int level, 
				const time_t curTime, 
				struct tm *ct, 
				const char *fmt, 
				va_list args
			);
			static void addTarget(Target *target);
			static void clearTargets();
		private:
			typedef std::vector<Target*> TTargetList;
			static TTargetList _targets;
			static ScreenTarget _defaultTarget;
			static const char *TAG;
		};
		
		template<int level, int logLevel>
		struct NeedLog
		{
			enum
			{
				IS_NEEDED = level <=  logLevel,
			};
		};
		
		template<bool needLogging, int level, class TLogSystem>
		class Log
		{
		};
		
		template<int level, class TLogSystem>
		class Log<true, level, TLogSystem>
		{
		public:
			static inline void L(const char *fmt, ...)
			{
				time_t curTime = time(NULL);
				struct tm *ct = localtime(&curTime);
				
				va_list args;
				for (int target = 0; ; target++)
				{
					va_start(args, fmt);
					bool res = TLogSystem::log(target, level, curTime, ct, fmt, args);
					va_end(args);
					
					if (!res)
						break;
				}
			};
		};
		
		template<int level, class TLogSystem> // no need logging on current level
		class Log<false, level, TLogSystem>
		{
		public:
			static inline void L(const char *fmt, ...)
			{
				 
			}
		};
		
		typedef Log<NeedLog<ELogLevel::INFO, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::INFO, LogSystem> Info;
		typedef Log<NeedLog<ELogLevel::WARNING, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::WARNING, LogSystem> Warning;
		typedef Log<NeedLog<ELogLevel::ERROR, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::ERROR, LogSystem> Error;
		typedef Log<NeedLog<ELogLevel::FATAL, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::FATAL, LogSystem> Fatal;
	};
};

#endif //__FL_LOG_HPP__
