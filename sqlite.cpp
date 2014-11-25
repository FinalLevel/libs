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
#include "log.hpp"
#include "mysql.hpp"

using namespace fl::db;

SQLite::SQLite()
	: _conn(NULL)
{
}


bool SQLite::open(const char * const filename, const int flags)
{
	sqlite3 *conn = NULL;
	if (sqlite3_open_v2(filename, &conn, flags, NULL) == SQLITE_OK) {
		_filename = filename;
		_conn.reset(conn, sqlite3_close);
		return true;
	} else {
		_conn.reset(conn, sqlite3_close);
		_filename.clear();
		log::Error::L("Can't open SQLite db %s (%s)\n", filename, _conn.get() ? sqlite3_errmsg(_conn.get()) : "_conn is NULL");
		return false;
	}
}

class SQLiteStatement SQLite::createStatement(const BString &sql)
{
	return SQLiteStatement(_conn, sql.c_str(), sql.size());
}

class SQLiteStatement SQLite::createStatement(const char * const sql)
{
	return SQLiteStatement(_conn, sql, strlen(sql));
}

class SQLiteStatement SQLite::createStatement(const std::string &sql)
{
	return SQLiteStatement(_conn, sql.c_str(), sql.size());
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

SQLiteStatement::SQLiteStatement(TSQLiteDescriptorSharedPtr &conn, const char * const sql, const size_t size)
	: _conn(conn), _ppStmt(NULL)
{
	int res = sqlite3_prepare_v2(_conn.get(), sql, size,  &_ppStmt, NULL);
	if (res != SQLITE_OK) {
		log::Error::L("SQLiteStatement error [%s] %d (%s)\n", sql, res, sqlite3_errmsg(_conn.get()));
		sqlite3_finalize(_ppStmt);
		_ppStmt = NULL;
		throw Error(res);
	}
}

SQLiteStatement::~SQLiteStatement()
{
	sqlite3_finalize(_ppStmt);
}

SQLiteStatement::SQLiteStatement(SQLiteStatement &&stmt)
	: _conn(stmt._conn), _ppStmt(stmt._ppStmt)
{
	stmt._ppStmt = NULL;
}

SQLiteStatement &SQLiteStatement::operator= (SQLiteStatement &&src)
{
	std::swap(_conn, src._conn);
	std::swap(_ppStmt, src._ppStmt);
	return *this;
}

bool SQLiteStatement::execute()
{
	int res = sqlite3_step(_ppStmt);
	if (res == SQLITE_DONE) {
		return true;
	} else {
		log::Error::L("SQLiteStatement error %d (%s)\n", res, sqlite3_errmsg(_conn.get()));
		throw Error(res);
	}
}

bool SQLiteStatement::next()
{
	int res = sqlite3_step(_ppStmt);
	if (res == SQLITE_ROW) {
		return true;
	} else if (res == SQLITE_DONE) {
		return false;
	} else {
		log::Error::L("SQLiteStatement error %d (%s)\n", res, sqlite3_errmsg(_conn.get()));
		throw Error(res);
	}
}

void SQLiteStatement::reset()
{
	sqlite3_reset(_ppStmt);
}


const char *SQLiteStatement::get(const int iCol)
{
	return (const char*)(sqlite3_column_text(_ppStmt, iCol));
}

void SQLiteStatement::bind(const int iValue, const int val)
{
	int res = sqlite3_bind_int(_ppStmt, iValue, val);
	if (res != SQLITE_OK) {
		log::Error::L("Can't bind int to %d\n", iValue);
		throw Error(res);
	}
}

void SQLiteStatement::bind(const int iValue, const unsigned int val)
{
	int res = sqlite3_bind_int(_ppStmt, iValue, val);
	if (res != SQLITE_OK) {
		log::Error::L("Can't bind int to %d\n", iValue);
		throw Error(res);
	}
}

void SQLiteStatement::bind(const int iValue, const long int val)
{
	int res = sqlite3_bind_int64(_ppStmt, iValue, val);
	if (res != SQLITE_OK) {
		log::Error::L("Can't bind long int to %d\n", iValue);
		throw Error(res);
	}
}

void SQLiteStatement::bind(const int iValue, const double val)
{
	int res = sqlite3_bind_double(_ppStmt, iValue, val);
	if (res != SQLITE_OK) {
		log::Error::L("Can't bind double to %d\n", iValue);
		throw Error(res);
	}	
}

void SQLiteStatement::bind(const int iValue, const char * const text, const size_t length)
{
	int res = sqlite3_bind_text(_ppStmt, iValue, text, length, SQLITE_STATIC);
	if (res != SQLITE_OK) {
		log::Error::L("Can't bind text to %d\n", iValue);
		throw Error(res);
	}		
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
