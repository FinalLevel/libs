#pragma once
#ifndef __FL_LIBS_MYSQL_POOL_HPP
#define	__FL_LIBS_MYSQL_POOL_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: A Mysql connection pool implementation class
///////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <string>

#include "mysql.hpp"
#include "mutex.hpp"

namespace fl {
	namespace db {
		using fl::threads::Mutex;
		using fl::threads::AutoMutex;
			
		class MysqlPool
		{
		public:
			MysqlPool(const size_t connectionCount, const std::string &dbHost, std::string &dbUser, 
				const std::string &dbPassword, const std::string &dbName, const uint16_t dbPort = 0);
			~MysqlPool();
			
			
			MysqlPool(const MysqlPool&) = delete;
			MysqlPool(MysqlPool&&) = delete;
			
			class Connection
			{
			public:
				Connection(const Connection &conn) = delete;
				Connection &operator=(const Connection &conn) = delete;

				Connection& operator=(Connection &&conn)
				{
					std::swap(conn._autoSync, _autoSync);
					std::swap(conn._conn, _conn);
					std::swap(conn._query, _query);
					return *this;
				}
			private:
				friend class MysqlPool;
				Connection(AutoMutex &&sync, Mysql *conn, MysqlQuery *query)
					: _autoSync(std::move(sync)), _conn(conn), _query(query)
				{
				}
				Connection(Connection &&conn)
					: _autoSync(std::move(conn._autoSync)), _conn(conn._conn), _query(conn._query)
				{
					conn._conn = NULL;
					conn._query = NULL;
				}
				AutoMutex _autoSync;
				Mysql *_conn;
				MysqlQuery *_query;
			};
			
			Connection get(const uint32_t key);
		private:
			std::string _dbHost;
			std::string _dbUser;
			std::string _dbPassword;
			std::string _dbName;
			uint16_t _dbPort;
			
			typedef std::vector<Mysql*> TMysqlVector;
			TMysqlVector _connections;
			typedef std::vector<MysqlQuery> TMysqlQueryVector;
			TMysqlQueryVector _queryBuffs;
			typedef std::vector<Mutex*> TMutexVector;
			TMutexVector _syncs;
		};
	};
};

#endif	// __FL_LIBS_MYSQL_POOL_HPP
