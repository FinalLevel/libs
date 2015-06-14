///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Mysql wrapper class
///////////////////////////////////////////////////////////////////////////////

#include "mysql.hpp"
#include "db_log.hpp"


using namespace fl::db;

std::string Mysql::_characterSet("utf8");

void Mysql::setDefaultCharacterSet(const char *characterSet)
{
	_characterSet = characterSet;
}

Mysql::Mysql()
	: _mysql(mysql_init(NULL), mysql_close)
{
	if (! _mysql) {
		log::Fatal::L("Mysql initialization has failed\n");
		throw MysqlError("Mysql initialization has failed");
	}
}

bool Mysql::connect(const char *hostName, const char *userName, const char *password, const char *dbName, 
	const unsigned int port, const char *socketName, const unsigned int flags)
{
	if (mysql_real_connect (_mysql.get(), hostName, userName, password, dbName, port, socketName, flags) == NULL) 	{
		log::Error::L("Cannot connect to Mysql server: %s (%d)\n", mysql_error(_mysql.get()), mysql_errno(_mysql.get()));
		return false;
	}
#if (MYSQL_VERSION_ID > 50000)
	mysql_options(_mysql.get(), MYSQL_OPT_RECONNECT, "true");
#endif
	return setCharacterSet(_characterSet.c_str());
}

bool Mysql::setCharacterSet(const char *characterSet)
{
	if (mysql_set_character_set(_mysql.get(), characterSet)) {
		log::Error::L("Cannot set character set of Mysql: %s (%d) to %s\n", mysql_error(_mysql.get()), 
			mysql_errno(_mysql.get()), characterSet);
		return false;
	}
	else
		return true;
}

bool Mysql::setServerOption(const enum enum_mysql_set_option option)
{
	return (mysql_set_server_option(_mysql.get(), option) == 0);
}

const char *Mysql::getCharacterSet() const
{
	return mysql_character_set_name(_mysql.get());
}

void Mysql::_errorQuery(const char *operation, const char *queryStr, const unsigned long length)
{
	if (length < MAX_SHOW_ERROR_SQL) {
		log::Error::L("An error %s (%d) op:%s query:[%s]\n", mysql_error(_mysql.get()), mysql_errno(_mysql.get()),
			operation, queryStr);
	} else {
		auto showPartLength = MAX_SHOW_ERROR_SQL / 4;
		std::string queryStart(queryStr, showPartLength);
		std::string queryEnd(queryStr + (length - showPartLength), showPartLength);
		log::Error::L("An error %s (%d) op:%s query (%lu):[%s ... %s]\n", mysql_error(_mysql.get()), 
			mysql_errno(_mysql.get()), operation, length, queryStart.c_str(), queryEnd.c_str());
	}
}

TMysqlResultPtr Mysql::query(const char *queryStr)
{
	return query(queryStr, strlen(queryStr));
}

TMysqlResultPtr Mysql::query(const std::string &queryStr)
{
	return query(queryStr.c_str(), queryStr.size());
}

TMysqlResultPtr Mysql::query(const MysqlQuery &mysqlQuery)
{
	return query(mysqlQuery.c_str(), mysqlQuery.size());
}

bool Mysql::_repeatQuery(const char *queryStr, const unsigned long length)
{
	auto mysqlErrno = mysql_errno(_mysql.get());
	if ((mysqlErrno == CR_SERVER_GONE_ERROR) || (mysqlErrno == CR_SERVER_LOST))	{
		if (mysql_ping(_mysql.get()) == 0) { 
			log::Warning::L("Repeat query because of %s (%d)\n", mysql_error(_mysql.get()), mysqlErrno);
			if (mysql_real_query(_mysql.get(), queryStr, length) == 0)
				return true;
		}
	}
	return false;
}

TMysqlResultPtr Mysql::query(const char *queryStr, const unsigned long length)
{
	if (!execute(queryStr, length, "q"))
		return TMysqlResultPtr();

	MYSQL_RES *result = mysql_store_result(_mysql.get());
	if (result) {
		TMysqlResultPtr res(new MysqlResult(result));
		log::Info::L("q: [%s], r: %u\n", queryStr, res->numRows());
		return res;
	} else {
		_errorQuery("store_result", queryStr, length);
		return TMysqlResultPtr();
	}
}

TMysqlResultPtr Mysql::queryUse(const char *queryStr)
{
	return queryUse(queryStr, strlen(queryStr));
}

TMysqlResultPtr Mysql::queryUse(const MysqlQuery &mysqlQuery)
{
	return queryUse(mysqlQuery.c_str(), mysqlQuery.size());
}

TMysqlResultPtr Mysql::queryUse(const char *queryStr, const unsigned long length)
{
	if (!execute(queryStr, length, "qu"))
		return TMysqlResultPtr();

	MYSQL_RES *result = mysql_use_result(_mysql.get());
	if (result) {
		log::Info::L("qu: [%s]\n", queryStr);
		return TMysqlResultPtr(new MysqlResult(result));
	} else {
		_errorQuery("use_result", queryStr, length);
		return TMysqlResultPtr();
	}
}

bool Mysql::execute(const char *queryStr)
{
	return execute(queryStr, strlen(queryStr));
}

bool Mysql::execute(const MysqlQuery &mysqlQuery)
{
	return execute(mysqlQuery.c_str(), mysqlQuery.size());
}

bool Mysql::execute(const char *queryStr, const unsigned long length, const char *cmd)
{
	auto res = mysql_real_query(_mysql.get(), queryStr, length);
  if (res && !_repeatQuery(queryStr, length)) {
		_errorQuery(cmd, queryStr, length);
		return false;
	} else {
		if (cmd[0] == 'e') {
			log::Info::L("ex: [%s], ar: %u\n", queryStr, affectedRows());
		}
		return true;
	}
}

unsigned long Mysql::insertID() const
{
	return mysql_insert_id(_mysql.get());
}

unsigned long Mysql::affectedRows() const
{
	return mysql_affected_rows(_mysql.get());
}

void Mysql::addRealEscape(BString &buf, const char *value, const long length)
{
	auto currentSize = buf.size();
	currentSize += mysql_real_escape_string(_mysql.get(), buf.reserveBuffer(length * 2  + 1), value, length);
	buf.trim(currentSize);
}

void Mysql::addRealEscape(BString &buf, const std::string &value)
{
	addRealEscape(buf, value.c_str(), value.size());
}

void Mysql::addRealEscape(BString &buf, const BString &value)
{
	addRealEscape(buf, value.c_str(), value.size());
}


bool Mysql::nextResult()
{
	return (mysql_next_result(_mysql.get()) == 0);
}

MysqlQuery Mysql::createQuery(const BString::TSize reserved)
{
	return MysqlQuery(_mysql, reserved);
}


MysqlResult::MysqlResult(MYSQL_RES *result)
	: _result(result), _currentRow(NULL)
{
	
}

MysqlResult::~MysqlResult()
{
	mysql_free_result(_result);
}

bool MysqlResult::next()
{
	_currentRow = mysql_fetch_row(_result);
	return (_currentRow != NULL);
}

void MysqlResult::rewind()
{
	mysql_data_seek(_result, 0);
}

unsigned long MysqlResult::getFieldLength(const int pos)
{
	unsigned long *lengths = mysql_fetch_lengths(_result);
	return lengths[pos];
}

unsigned long MysqlResult::numRows()
{
	return mysql_num_rows(_result);
}

const char *MysqlResult::getFieldName(const int pos)
{
	auto field = mysql_fetch_field_direct(_result, pos);
	if (field)
		return field->name;
	else
		return NULL;
}


MysqlQuery::MysqlQuery(MysqlQuery &&src)
	: BString(std::move(src)), _mysql(src._mysql), _needEscape(src._needEscape)
{
	src._needEscape = false;
}

MysqlQuery &MysqlQuery::operator= (MysqlQuery &&src)
{
	BString::operator= (std::move(src));
	std::swap(_mysql, src._mysql);
	std::swap(_needEscape, src._needEscape);
	return *this;
}

MysqlQuery::MysqlQuery(TMysqlDescriptorSharedPtr &mysql, const BString::TSize reserved)
	: BString(reserved), _mysql(mysql), _needEscape(false)
{
	
}

void MysqlQuery::clear()
{
	BString::clear();
	_needEscape = false;
}

void MysqlQuery::_escape(const char *value, const size_t length)
{
	*this << '\'';
	auto currentSize = size();
	currentSize += mysql_real_escape_string(_mysql.get(), reserveBuffer(length * 2  + 1), value, length);
	trim(currentSize);
	*this << '\'';
}
