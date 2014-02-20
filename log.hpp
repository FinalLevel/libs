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
		
		typedef std::vector<Target*> TTargetList;
		
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
			LogSystem(const char *tag)
				: _tag(tag)
			{
			}
			~LogSystem()
			{
				clearTargets();
			}
			static bool log(
				const size_t target, 
				const int level, 
				const time_t curTime, 
				struct tm *ct, 
				const char *fmt, 
				va_list args
			); // empty function
			
			void addTarget(Target *target);
			void clearTargets();
		protected:
			bool _log(
				const size_t target, 
				const int level, 
				const time_t curTime, 
				struct tm *ct, 
				const char *fmt, 
				va_list args
			);
			TTargetList _targets;
			const char *_tag;
		};
		
		class LibLogSystem : public LogSystem
		{
		public:
			static bool log(
				const size_t target, 
				const int level, 
				const time_t curTime, 
				struct tm *ct, 
				const char *fmt, 
				va_list args
			)
			{
				static ScreenTarget screenTarget;
				screenTarget.log(level, "fLib", curTime, ct, fmt, args);
				return false;
			}
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
		
		typedef Log<NeedLog<ELogLevel::INFO, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::INFO, LibLogSystem> Info;
		typedef Log<NeedLog<ELogLevel::WARNING, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::WARNING, LibLogSystem> Warning;
		typedef Log<NeedLog<ELogLevel::ERROR, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::ERROR, LibLogSystem> Error;
		typedef Log<NeedLog<ELogLevel::FATAL, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::FATAL, LibLogSystem> Fatal;
	};
};

#endif //__FL_LOG_HPP__
