///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: SQLite wrapper class implementation
///////////////////////////////////////////////////////////////////////////////


#include "sqlite.hpp"
#include "db_log.hpp"
#include "timer.hpp"

using namespace fl::db;
using fl::chrono::Timer;

SQLite::SQLite()
	: _conn(NULL)
{
}


bool SQLite::open(const char * const filename, const int flags)
{
	Timer timer;
	sqlite3 *conn = NULL;
	if (sqlite3_open_v2(filename, &conn, flags, NULL) == SQLITE_OK) {
		_filename = filename;
		_conn.reset(conn, sqlite3_close);
		log::Info::L("Open SQLite db %s (%llums)\n", _filename.c_str(), timer.elapsed().count());
		return true;
	} else {
		_conn.reset(conn, sqlite3_close);
		_filename.clear();
		log::Error::L("Can't open SQLite db %s (%s)\n", filename, _conn.get() ? sqlite3_errmsg(_conn.get()) : "_conn is NULL");
		return false;
	}
}

class SQLiteStatement SQLite::createStatement(const BString &sql, BString* bind)
{
	return SQLiteStatement(_conn, sql.c_str(), sql.size(), bind);
}

class SQLiteStatement SQLite::createStatement(const char * const sql, BString* bind)
{
	return SQLiteStatement(_conn, sql, strlen(sql), bind);
}

class SQLiteStatement SQLite::createStatement(const std::string &sql, BString* bind)
{
	return SQLiteStatement(_conn, sql.c_str(), sql.size(), bind);
}

bool SQLite::execute(const BString &query)
{
	SQLiteStatement sql(_conn, query.c_str(), query.size());
	return sql.execute();
}

sqlite3_int64 SQLite::insertId()
{
	return sqlite3_last_insert_rowid(_conn.get());
}

int SQLite::affectedRows()
{
	return sqlite3_changes(_conn.get());
}

SQLiteAutoRollbackTransaction SQLite::startAutoRollbackTransaction()
{
	return SQLiteAutoRollbackTransaction(_conn);
}

SQLiteAutoRollbackTransaction::SQLiteAutoRollbackTransaction(SQLiteAutoRollbackTransaction &&transaction)
	: _conn(transaction._conn), _rollback(transaction._rollback)
{
	transaction._rollback = false;
	transaction._conn.reset();
}

SQLiteAutoRollbackTransaction &SQLiteAutoRollbackTransaction::operator= (SQLiteAutoRollbackTransaction &&src)
{
	std::swap(src._conn, _conn);
	_rollback = src._rollback;
	src._rollback = false;
	src._conn.reset();
	return *this;
}

SQLiteAutoRollbackTransaction::SQLiteAutoRollbackTransaction(TSQLiteDescriptorSharedPtr &conn)
	: _conn(conn)
{
	static const std::string BEGIN_TRANSACTION { "BEGIN TRANSACTION" };
	SQLiteStatement sql(_conn, BEGIN_TRANSACTION.c_str(), BEGIN_TRANSACTION.size());
	if (!sql.execute()) {
		throw TransactionError();
	}
}

SQLiteAutoRollbackTransaction::~SQLiteAutoRollbackTransaction()
{
	if (_rollback) {
		static const std::string ROLLBACK_TRANSACTION { "ROLLBACK TRANSACTION" };
		SQLiteStatement sql(_conn, ROLLBACK_TRANSACTION.c_str(), ROLLBACK_TRANSACTION.size());
		if (!sql.execute()) {
			throw TransactionError();
		}
	}
}

void SQLiteAutoRollbackTransaction::commit()
{
	if (_rollback) {
		static const std::string COMMIT_TRANSACTION { "COMMIT TRANSACTION" };
		SQLiteStatement sql(_conn, COMMIT_TRANSACTION.c_str(), COMMIT_TRANSACTION.size());
		if (!sql.execute()) {
			throw TransactionError();
		}
		_rollback = false;
	} else {
		throw TransactionError();
	}
}

SQLiteStatement::SQLiteStatement(TSQLiteDescriptorSharedPtr &conn, const char * const sql, const size_t size, BString* bind)
	: _conn(conn), _ppStmt(NULL), _firstRow(true), _bindValues(bind)
{
	int res = sqlite3_prepare_v2(_conn.get(), sql, size,  &_ppStmt, NULL);
	if (res != SQLITE_OK) {
		log::Error::L("EStmt [%s] %d (%s)\n", sql, res, sqlite3_errmsg(_conn.get()));
		sqlite3_finalize(_ppStmt);
		_ppStmt = NULL;
		throw Error(res);
	}
	if (_bindValues) {
		_bindValues->clear();
	}
}

SQLiteStatement::~SQLiteStatement()
{
	sqlite3_finalize(_ppStmt);
}

SQLiteStatement::SQLiteStatement(SQLiteStatement &&stmt)
	: _conn(stmt._conn), _ppStmt(stmt._ppStmt), _firstRow(stmt._firstRow), _bindValues(stmt._bindValues)
{
	stmt._ppStmt = NULL;
	stmt._firstRow = true;
	if (stmt._bindValues) {
		stmt._bindValues->clear();
	}
}

SQLiteStatement &SQLiteStatement::operator= (SQLiteStatement &&src)
{
	std::swap(_conn, src._conn);
	std::swap(_ppStmt, src._ppStmt);
	std::swap(_bindValues, src._bindValues);
	std::swap(_firstRow, src._firstRow);
	return *this;
}

int SQLiteStatement::affectedRows()
{
	return sqlite3_changes(_conn.get());
}

bool SQLiteStatement::execute()
{
	Timer timer;
	int res = sqlite3_step(_ppStmt);
	const char* sqlString = sqlite3_sql(_ppStmt);
	size_t len = strlen(sqlString);
	if (res == SQLITE_DONE) {
		if (_bindValues && !_bindValues->empty()) {
			if(len > 100) {
				log::Info::L("e:[%.50s...%s] v:[%s] r:%d (%llums)\n", sqlString, sqlString + len - 50, _bindValues->c_str(), affectedRows(), timer.elapsed().count());
			} else {
				log::Info::L("e:[%s] v:[%s] r:%d (%llums)\n", sqlString, _bindValues->c_str(), affectedRows(), timer.elapsed().count());
			}
		} else {
			if(len > 100) {
				log::Info::L("e:[%.50s...%s] r:%d (%llums)\n", sqlString, sqlString + len - 50, affectedRows(), timer.elapsed().count());
			} else {
				log::Info::L("e:[%s] r:%d (%llums)\n", sqlString, affectedRows(), timer.elapsed().count());
			}
		}
		return true;
	} else {
		if (_bindValues && !_bindValues->empty()) {
			if(len > 100) {
				log::Info::L("EStmt [%.50s...%s] v:[%s] %d (%s)\n", sqlString, sqlString + len - 50, _bindValues->c_str(), res, sqlite3_errmsg(_conn.get()));
			} else {

				log::Error::L("EStmt [%s] v:[%s] %d (%s)\n", sqlString, _bindValues->c_str(), res, sqlite3_errmsg(_conn.get()));
			}
		} else {
			if(len > 100) {
				log::Info::L("EStmt [%.50s...%s] %d (%s)\n", sqlString, sqlString + len - 50, res, sqlite3_errmsg(_conn.get()));
			} else {

				log::Error::L("EStmt [%s] %d (%s)\n", sqlString, res, sqlite3_errmsg(_conn.get()));
			}
		}
		throw Error(res);
	}
}

bool SQLiteStatement::next()
{
	Timer timer;
	int res = sqlite3_step(_ppStmt);
	const char* sqlString = sqlite3_sql(_ppStmt);
	size_t len = strlen(sqlString);
	if (res == SQLITE_ROW) {
		if (_firstRow) {
			if (_bindValues && !_bindValues->empty()) {
				if(len > 100) {
					log::Info::L("q:[%.50s ... %s] v:[%s] (%llums)\n", sqlString, sqlString + len - 50, _bindValues->c_str(), timer.elapsed().count());
				} else {
					log::Info::L("q:[%s] v:[%s] (%llums)\n", sqlString, _bindValues->c_str(), timer.elapsed().count());
				}
			} else {
				if(len > 100) {
					log::Info::L("q:[%.50s ... %s] (%llums)\n", sqlString, sqlString + len - 50, timer.elapsed().count());
				} else {
					log::Info::L("q:[%s] (%llums)\n", sqlString, timer.elapsed().count());
				}
			}
			_firstRow = false;
		}
		return true;
	} else if (res == SQLITE_DONE) {
		return false;
	} else {
		if (_bindValues && !_bindValues->empty()) {
			if(len > 100) {
				log::Info::L("EStmt [%.50s ... %s] v:[%s] %d (%s)\n", sqlString, sqlString + len - 50, _bindValues->c_str(), res, sqlite3_errmsg(_conn.get()));
			} else {

				log::Error::L("EStmt [%s] v:[%s] %d (%s)\n", sqlString, _bindValues->c_str(), res, sqlite3_errmsg(_conn.get()));
			}
		} else {
			if(len > 100) {
				log::Info::L("EStmt [%.50s ... %s] %d (%s)\n", sqlString, sqlString + len - 50, res, sqlite3_errmsg(_conn.get()));
			} else {

				log::Error::L("EStmt [%s] %d (%s)\n", sqlString, res, sqlite3_errmsg(_conn.get()));
			}
		}
		throw Error(res);
	}
}


void SQLiteStatement::reset()
{
	if (_bindValues) {
		_bindValues->clear();
	}
	_firstRow = true;
	sqlite3_reset(_ppStmt);
}


const char *SQLiteStatement::get(const int iCol)
{
	return (const char*)(sqlite3_column_text(_ppStmt, iCol));
}

int SQLiteStatement::length(const int iCol)
{
	return sqlite3_column_bytes(_ppStmt, iCol);
}

void SQLiteStatement::bind(const int iValue, const int val)
{
	int res = sqlite3_bind_int(_ppStmt, iValue, val);
	if (res != SQLITE_OK) {
		log::Error::L("EStmt [%s] Can't bind int to %d\n", sqlite3_sql(_ppStmt), iValue);
		throw Error(res);
	}
	addBoundedValue(val);
}

void SQLiteStatement::bind(const int iValue, const uint32_t val)
{
	int res = sqlite3_bind_int(_ppStmt, iValue, val);
	if (res != SQLITE_OK) {
		log::Error::L("EStmt [%s] Can't bind int to %d\n", sqlite3_sql(_ppStmt), iValue);
		throw Error(res);
	}
	addBoundedValue(val);
}

void SQLiteStatement::bind(const int iValue, const uint64_t val)
{
	int res = sqlite3_bind_int64(_ppStmt, iValue, val);
	if (res != SQLITE_OK) {
		log::Error::L("EStmt [%s] Can't bind uint64_t to %d\n", sqlite3_sql(_ppStmt), iValue);
		throw Error(res);
	}
	addBoundedValue(val);
}

void SQLiteStatement::bind(const int iValue, const long int val)
{
	int res = sqlite3_bind_int64(_ppStmt, iValue, val);
	if (res != SQLITE_OK) {
		log::Error::L("EStmt [%s] Can't bind long int to %d\n", sqlite3_sql(_ppStmt), iValue);
		throw Error(res);
	}
	addBoundedValue(val);
}

void SQLiteStatement::bind(const int iValue, const double val)
{
	int res = sqlite3_bind_double(_ppStmt, iValue, val);
	if (res != SQLITE_OK) {
		log::Error::L("EStmt [%s] Can't bind double to %d\n", sqlite3_sql(_ppStmt), iValue);
		throw Error(res);
	}
	addBoundedValue(val);
}

void SQLiteStatement::bind(const int iValue, const char * const text, const size_t length)
{
	int res = sqlite3_bind_text(_ppStmt, iValue, text, length, SQLITE_STATIC);
	if (res != SQLITE_OK) {
		log::Error::L("EStmt [%s] Can't bind text to %d\n", sqlite3_sql(_ppStmt), iValue);
		throw Error(res);
	}
	addBoundedValue(text, length);
}

void SQLiteStatement::bindBlob(const int iValue, const void* data, const size_t length)
{
	int res = sqlite3_bind_blob(_ppStmt, iValue, data, length, SQLITE_STATIC);
	if (res != SQLITE_OK) {
		log::Error::L("EStmt [%s] Can't bind text to %d\n", sqlite3_sql(_ppStmt), iValue);
		throw Error(res);
	}
	addBoundedValue("(BLOB)");
}

void SQLiteStatement::bind(const int iValue, const uint8_t * const data, const size_t length)
{
	bind(iValue, reinterpret_cast<const char* const>(data), length);
}

void SQLiteStatement::bind(const int iValue, const std::string &val)
{
	bind(iValue, val.c_str(), val.size());
}

void SQLiteStatement::bind(const int iValue, const BString &val)
{
	bind(iValue, val.c_str(), val.size());
}

void SQLiteStatement::bind(const int iValue, const char * const text)
{
	bind(iValue, text, strlen(text));
}
