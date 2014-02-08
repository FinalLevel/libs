#pragma once
#ifndef __FL_LOGGER_HPP__
#define __FL_LOGGER_HPP__

///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under GNU GENERAL PUBLIC LICENSE, Version 2.0. (See
// accompanying file LICENSE or copy at
// https://www.gnu.org/licenses/gpl-2.0.en.html)
//
///////////////////////////////////////////////////////////////////////////////

#ifndef FL_LOG_LEVEL // set default log level to WARNING
#define FL_LOG_LEVEL 2
#endif

#include <cstdarg>
#include <ctime>
#include <cstdio>
#include <vector>

namespace fl
{
	namespace log
	{
		namespace ELogLevel
		{
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
			virtual void log(const int level, const time_t curTime, struct tm *ct, const char *fmt, va_list args) = 0;
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
			virtual void log(const int level, const time_t curTime, struct tm *ct, const char *fmt, va_list args);
		};
		
		class FileTarget : public Target
		{
		public:
			FileTarget(const char *fileName);
			virtual ~FileTarget();
			virtual void log(const int level, const time_t curTime, struct tm *ct, const char *fmt, va_list args);
		private:
			FILE *_fd;
		};
		
		class LogSystem
		{
		public:
			static bool log(const size_t target, const int level, const char *fmt, va_list args);
			static void addTarget(Target *target);
		private:
			typedef std::vector<Target*> TTargetList;
			static TTargetList _targets;
			static ScreenTarget _defaultTarget;
		};
		
		template<int level>
		struct NeedLog
		{
			enum
			{
				IS_NEEDED = level <=  FL_LOG_LEVEL,
			};
		};
		
		template<bool needLogging, int level>
		class Log
		{
		};
		
		template<int level>
		class Log<true, level>
		{
		public:
			static inline void L(const char *fmt, ...)
			{
				va_list args;
				for (int target = 0; ; target++)
				{
					va_start(args, fmt);
					bool res = LogSystem::log(target, level, fmt, args);
					va_end(args);
					
					if (!res)
						break;
				}
			};
		};
		
		template<int level> // no need logging on current level
		class Log<false, level>
		{
		public:
			static inline void L(const char *fmt, ...)
			{
				 
			}
		};
		
		typedef Log<NeedLog<ELogLevel::INFO>::IS_NEEDED, ELogLevel::INFO> Info;
		typedef Log<NeedLog<ELogLevel::WARNING>::IS_NEEDED, ELogLevel::WARNING> Warning;
		typedef Log<NeedLog<ELogLevel::ERROR>::IS_NEEDED, ELogLevel::ERROR> Error;
		typedef Log<NeedLog<ELogLevel::FATAL>::IS_NEEDED, ELogLevel::FATAL> Fatal;
	};
};

#endif //__FL_LOGGER_HPP__
