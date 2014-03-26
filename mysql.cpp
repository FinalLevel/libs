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
#include "log.hpp"


using namespace fl::db;

std::string Mysql::_characterSet("UTF-8");

void Mysql::setDefaultCharset(const char *characterSet)
{
	_characterSet = characterSet;
}

Mysql::Mysql()
{
	_connection = mysql_init (NULL);
	if (_connection == NULL) {
		log::Fatal::L("Mysql initialization has failed\n");
		throw MysqlError("Mysql initialization has failed");
	}
}

Mysql::~Mysql()
{
	mysql_close (_connection);
}

bool Mysql::connect(const char *hostName, const char *userName, const char *password, const char *dbName, 
	const unsigned int port, const char *socketName, const unsigned int flags)
{
	if (mysql_real_connect (_connection, hostName, userName, password, dbName, port, socketName, flags) == NULL) 	{
		log::Error::L("Cannot connect to Mysql server %d:%s\n", mysql_errno(_connection), mysql_error(_connection));
		return false;
	}
#if (MYSQL_VERSION_ID > 50000)
	mysql_options(_connection, MYSQL_OPT_RECONNECT, "true");
#endif
	return setCharset(_characterSet.c_str());
}

bool Mysql::setCharset(const char *characterSet)
{
	if (mysql_set_character_set(_connection, characterSet)) {
		log::Error::L("Cannot set character set of Mysql %d:%s to %s\n", mysql_errno(_connection), 
			mysql_error(_connection), characterSet);
		return false;
	}
	else
		return true;
}
