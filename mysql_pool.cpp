///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: A Mysql connection pool implementation class
///////////////////////////////////////////////////////////////////////////////

#include <memory>
#include "mysql_pool.hpp"

using namespace fl::db;

MysqlPool::MysqlPool(const size_t connectionCount, const std::string &dbHost, const std::string &dbUser, 
	const std::string &dbPassword, const std::string &dbName, const uint16_t dbPort)
	: _dbHost(dbHost), _dbUser(dbUser), _dbPassword(dbPassword), _dbName(dbName), _dbPort(dbPort),
		_connections(connectionCount, NULL)
{
	if (connectionCount == 0)
		throw MysqlError("Connection pool can't be empty");
	
	for (size_t i = 0; i < connectionCount; i++) {
		_syncs.push_back(new Mutex());
	}
}

MysqlPool::~MysqlPool()
{
	for (auto conn = _connections.begin(); conn != _connections.end(); conn++) {
		delete *conn;
	}
	for (auto sync = _syncs.begin(); sync != _syncs.end(); sync++) {
		delete *sync;
	}
}


void MysqlPool::_createConnection(size_t idx)
{
	std::unique_ptr<Mysql> conn(new Mysql());
	if (!conn->connect(_dbHost.c_str(), _dbUser.c_str(), _dbPassword.c_str(), _dbName.c_str(), _dbPort))
		throw MysqlError("MysqlPool can't establish connection to DB");
	log::Info::L("MysqlPool: Connection to %s:%u has been established\n", _dbHost.c_str(), _dbPort);
	while (_queryBuffs.size() <= idx) {
		_queryBuffs.push_back(conn->createQuery());
	}
	_connections[idx] = conn.release();
}

MysqlPool::Connection MysqlPool::get()
{
	AutoMutex autoMutex;
	for (size_t idx = 0; idx < _connections.size(); idx++) {
		if (autoMutex.tryLock(_syncs[idx])) {
			if (!_connections[idx]) {
				_createConnection(idx);
			}
			return Connection(std::move(autoMutex), _connections[idx], &_queryBuffs[idx]);
		}
	}
	autoMutex.lock(_syncs[0]);
	if (!_connections[0]) {
		_createConnection(0);
	}
	return Connection(std::move(autoMutex), _connections[0], &_queryBuffs[0]);
}

MysqlPool::Connection MysqlPool::getByKey(const uint32_t key)
{
	auto idx = key % _connections.size();
	AutoMutex autoMutex(_syncs[idx]);
	if (!_connections[idx]) {
		_createConnection(idx);
	}
	return Connection(std::move(autoMutex), _connections[idx], &_queryBuffs[idx]);
}

