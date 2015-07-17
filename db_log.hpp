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
			
			enum DBType {
				ALL,
				MYSQL,
				SQLITE,
				MAX
			};

			static const char* dbType[MAX] = {
				"DB",
				"DM",
				"DL"
			};

			template <DBType Type>
			class DbLogSystem
			{
			public:
				static bool log(const size_t target, const int level, const time_t curTime, struct tm *ct, const char *fmt, 
					va_list args)
				{
					return LogSystem::defaultLog().log(target, level, dbType[Type], curTime, ct, fmt, args);
				}
			};
			using fl::log::Log;
			using fl::log::NeedLog;
			using namespace fl::log::ELogLevel;

			#ifndef PRINT_DB_TYPE
			typedef Log<NeedLog<ELogLevel::INFO, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::INFO, DbLogSystem<ALL>> Info;
			typedef Log<NeedLog<ELogLevel::WARNING, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::WARNING, DbLogSystem<ALL>> Warning;
			typedef Log<NeedLog<ELogLevel::ERROR, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::ERROR, DbLogSystem<ALL>> Error;
			typedef Log<NeedLog<ELogLevel::FATAL, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::FATAL, DbLogSystem<ALL>> Fatal;
			#endif

			#ifdef DB_MYSQL
			typedef Log<NeedLog<ELogLevel::INFO, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::INFO, DbLogSystem<MYSQL>> Info;
			typedef Log<NeedLog<ELogLevel::WARNING, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::WARNING, DbLogSystem<MYSQL>> Warning;
			typedef Log<NeedLog<ELogLevel::ERROR, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::ERROR, DbLogSystem<MYSQL>> Error;
			typedef Log<NeedLog<ELogLevel::FATAL, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::FATAL, DbLogSystem<MYSQL>> Fatal;
			#endif

			#ifdef DB_SQLITE
			typedef Log<NeedLog<ELogLevel::INFO, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::INFO, DbLogSystem<SQLITE>> Info;
			typedef Log<NeedLog<ELogLevel::WARNING, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::WARNING, DbLogSystem<SQLITE>> Warning;
			typedef Log<NeedLog<ELogLevel::ERROR, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::ERROR, DbLogSystem<SQLITE>> Error;
			typedef Log<NeedLog<ELogLevel::FATAL, FL_LOG_LEVEL>::IS_NEEDED, ELogLevel::FATAL, DbLogSystem<SQLITE>> Fatal;
			#endif

		};	
	};
};

#endif	// __FL_DB_LOG_HPP
