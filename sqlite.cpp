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
	return SQLiteStatement(_conn, sql);
}

bool SQLite::execute(const BString &query)
{
	SQLiteStatement sql(_conn, query);
	return sql.execute();
}

SQLiteStatement::SQLiteStatement(TSQLiteDescriptorSharedPtr &conn, const BString &sql)
	: _conn(conn), _ppStmt(NULL)
{
	int res = sqlite3_prepare_v2(_conn.get(), sql.c_str(), sql.size(),  &_ppStmt, NULL);
	if (res != SQLITE_OK) {
		log::Error::L("SQLiteStatement error [%s] %d (%s)\n", sql.c_str(), res, sqlite3_errmsg(_conn.get()));
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

void SQLiteStatement::bind(const int iValue, const double val)
{
	int res = sqlite3_bind_double(_ppStmt, iValue, val);
	if (res != SQLITE_OK) {
		log::Error::L("Can't bind double to %d\n", iValue);
		throw Error(res);
	}	
}

void SQLiteStatement::_bindText(const int iValue, const char * const text, const size_t length)
{
	int res = sqlite3_bind_text(_ppStmt, iValue, text, length, SQLITE_STATIC);
	if (res != SQLITE_OK) {
		log::Error::L("Can't bind text to %d\n", iValue);
		throw Error(res);
	}		
}
void SQLiteStatement::bind(const int iValue, const std::string &val)
{
	_bindText(iValue, val.c_str(), val.size());
}

void SQLiteStatement::bind(const int iValue, const BString &val)
{
	_bindText(iValue, val.c_str(), val.size());
}

void SQLiteStatement::bind(const int iValue, const char * const text)
{
	_bindText(iValue, text, strlen(text));
}
