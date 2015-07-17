#pragma once
#ifndef __FL_DB_LOG_HPP
#define	__FL_DB_LOG_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Db's log customization class 
///////////////////////////////////////////////////////////////////////////////

#include "log.hpp"

namespace fl {
	namespace db {
		namespace log {
			using fl::log::Target;
			using fl::log::TTargetList;
			using fl::log::LogSystem;
			
			class DbLogSystem
			{
			public:
				static bool log(const size_t target, const int level, const time_t curTime, struct tm *ct, const char *fmt, 
					va_list args)
				{
					return LogSystem::defaultLog().log(target, level, "DB", curTime, ct, fmt, args);
				}
			};
			using fl::log::Log;
			using fl::log::NeedLog;
			using namespace fl::log::ELogLevel;
			
			typedef Log<NeedLog<ELogLevel::INFO, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::INFO, DbLogSystem> Info;
			typedef Log<NeedLog<ELogLevel::WARNING, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::WARNING, DbLogSystem> Warning;
			typedef Log<NeedLog<ELogLevel::ERROR, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::ERROR, DbLogSystem> Error;
			typedef Log<NeedLog<ELogLevel::FATAL, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::FATAL, DbLogSystem> Fatal;
		};	
	};
};

#endif	// __FL_DB_LOG_HPP
