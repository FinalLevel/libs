///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: SQLite class unit tests
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include <string>
#include "sqlite.hpp"

using namespace fl::db;

struct EmptyBaseFixture
{
	EmptyBaseFixture()
	{
		if (!sql.open("/tmp/testBase.db", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE)) {
			
			throw std::exception();
		}
	}
	~EmptyBaseFixture()
	{
		unlink(sql.filename().c_str());
	}
	SQLite sql;
};

BOOST_AUTO_TEST_SUITE( SQLiteTest )

BOOST_AUTO_TEST_CASE( SQLiteOpen )
{
	BOOST_CHECK_NO_THROW(EmptyBaseFixture());
}

BOOST_FIXTURE_TEST_CASE( SQLiteBasicSQLOperations, EmptyBaseFixture )
{
	try
	{
		BString query;
		query << "CREATE TABLE IF NOT EXISTS test (id INT PRIMARY KEY NOT NULL, name TEXT NOT NULL);";
		BOOST_REQUIRE(sql.execute(query));
		
		query.clear();
		enum ETestWildcats
		{
			WLD_ID = 1,
			WLD_NAME
		};
		query << "INSERT INTO test (id, name) VALUES (?,?);";
		auto res = sql.createStatement(query);
		res.bind(WLD_ID, 1);
		const char * testName1 = "test_name1";
		res.bind(WLD_NAME, testName1);
		BOOST_REQUIRE(res.execute());
		res.reset();
		res.bind(WLD_ID, 2);
		const char * testName2 = "test_name2";
		res.bind(WLD_NAME, testName2);
		BOOST_REQUIRE(res.execute());
		
		query.clear();
		enum ETestFlds
		{
			FLD_ID = 0,
			FLD_NAME
		};
		query << "SELECT id, name FROM test";
		auto selectRes = sql.createStatement(query);
		int idx = 0;
		while (selectRes.next()) {
			BOOST_REQUIRE((idx + 1) == selectRes.get<int>(FLD_ID));
			if (idx == 0) {
				BOOST_REQUIRE(strcmp(testName1, selectRes.get(FLD_NAME)) == 0);
			} else {
				BOOST_REQUIRE(strcmp(testName2, selectRes.get(FLD_NAME)) == 0);
			}
			idx++;
		}
		BOOST_REQUIRE(idx == 2);
	}
	catch (...)
	{
		BOOST_CHECK_NO_THROW(throw);
	}
}

BOOST_FIXTURE_TEST_CASE( SQLiteBinaryFields, EmptyBaseFixture )
{
	BString query;
	query << "CREATE TABLE test (bin BINARY(32) PRIMARY KEY NOT NULL)";
	BOOST_REQUIRE(sql.execute(query));
	
	std::string binData(32, ' ');
	binData.at(0) = 0;
	binData.at(10) = 2;
	
	query.clear();
	query << "INSERT INTO test (bin) VALUES(?)";
	auto st = sql.createStatement(query);
	st.bind(1, binData);
	BOOST_REQUIRE(st.execute());
	
	query.clear();
	query << "SELECT 1 FROM test WHERE bin=?";
	auto selectRes = sql.createStatement(query);
	selectRes.bind(1, binData);
	BOOST_REQUIRE(selectRes.next());
	selectRes.reset();
	
	binData.at(2) = '"';
	binData.at(3) = '\'';
	selectRes.bind(1, binData);
	BOOST_REQUIRE(selectRes.next() == false);
}

BOOST_AUTO_TEST_SUITE_END()
