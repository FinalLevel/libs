#pragma once
#ifndef __FL_MYSQL_HPP
#define	__FL_MYSQL_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Mysql wrapper class
///////////////////////////////////////////////////////////////////////////////

#include <mysql.h>
#include <errmsg.h>
#include <string>
#include "exception.hpp"

namespace fl {
	namespace db {
		class MysqlError : public fl::exceptions::Error
		{
		public:
			MysqlError(const char *what)
				: Error(what)
			{
			}
		};
		
		class Mysql
		{
		public:
			Mysql();
			~Mysql();
			bool connect(const char *hostName, const char *userName, const char *password, const char *dbName, 
				const unsigned int port, const char *socketName=NULL, const unsigned int flags = 0);
			
			bool setCharset(const char *characterSet);
			static void setDefaultCharset(const char *characterSet);
		private:
			static std::string _characterSet;
			MYSQL *_connection;
		};
	};
};

#endif	// __FL_MYSQL_HPP
