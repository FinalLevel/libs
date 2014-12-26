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
using fl::strings::CLR;

struct EmptyBaseFixture
{
	EmptyBaseFixture()
	{
		if (!db.open("/tmp/testBase.db", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE)) {
			
			throw std::exception();
		}
	}
	~EmptyBaseFixture()
	{
		unlink(db.filename().c_str());
	}
	SQLite db;
	BString query;
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
		query << CLR << "CREATE TABLE IF NOT EXISTS test (id INT PRIMARY KEY NOT NULL, name TEXT NOT NULL);";
		BOOST_REQUIRE(db.execute(query));
		
		enum ETestWildcats
		{
			WLD_ID = 1,
			WLD_NAME
		};
		query << CLR << "INSERT INTO test (id, name) VALUES (?,?);";
		auto res = db.createStatement(query);
		res.bind(WLD_ID, 1);
		const char * testName1 = "test_name1";
		res.bind(WLD_NAME, testName1);
		BOOST_REQUIRE(res.execute());
		res.reset();
		res.bind(WLD_ID, 2);
		const char * testName2 = "test_name2";
		res.bind(WLD_NAME, testName2);
		BOOST_REQUIRE(res.execute());
		
		enum ETestFlds
		{
			FLD_ID = 0,
			FLD_NAME
		};
		query << CLR << "SELECT id, name FROM test";
		auto selectRes = db.createStatement(query);
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
	query << CLR << "CREATE TABLE test (bin BINARY(32) PRIMARY KEY NOT NULL)";
	BOOST_REQUIRE(db.execute(query));
	
	std::string binData(32, ' ');
	binData.at(0) = 0;
	binData.at(10) = 2;
	
	query << CLR << "INSERT INTO test (bin) VALUES(?)";
	auto st = db.createStatement(query);
	st.bind(1, binData);
	BOOST_REQUIRE(st.execute());
	
	query << CLR << "SELECT 1 FROM test WHERE bin=?";
	auto selectRes = db.createStatement(query);
	selectRes.bind(1, binData);
	BOOST_REQUIRE(selectRes.next());
	selectRes.reset();
	
	binData.at(2) = '"';
	binData.at(3) = '\'';
	selectRes.bind(1, binData);
	BOOST_REQUIRE(selectRes.next() == false);
}

struct TestTableBaseFixture : public EmptyBaseFixture
{
	TestTableBaseFixture()
	{
		query << CLR << "CREATE TABLE test (id INT PRIMARY KEY NOT NULL)";
		db.execute(query);
	}
};

BOOST_FIXTURE_TEST_CASE( SQLiteAutoRollbackTransactionTest, TestTableBaseFixture )
{
	const int ROLLBACK_TEST_ID = 8;
	const int COMMITTED_TEST_ID = 7;
	try
	{
		auto transaction = db.startAutoRollbackTransaction();
		query << CLR << "INSERT INTO test (id) VALUES(?)";
		auto st = db.createStatement(query);
		st.bind(1, ROLLBACK_TEST_ID);
		BOOST_REQUIRE(st.execute());
	}
	catch (SQLiteAutoRollbackTransaction::TransactionError &er)
	{
		BOOST_CHECK_NO_THROW(throw);
	}
	
	try
	{
		auto transaction = db.startAutoRollbackTransaction();
		query << CLR << "INSERT INTO test (id) VALUES(?)";
		auto st = db.createStatement(query);
		st.bind(1, COMMITTED_TEST_ID);
		BOOST_REQUIRE(st.execute());
		transaction.commit();
	}
	catch (SQLiteAutoRollbackTransaction::TransactionError &er)
	{
		BOOST_CHECK_NO_THROW(throw);
	}
	
	query << CLR << "SELECT 1 FROM test WHERE id=?";
	auto selectRes = db.createStatement(query);
	selectRes.bind(1, ROLLBACK_TEST_ID);
	BOOST_REQUIRE(selectRes.next() == false);
	
	selectRes.reset();
	selectRes.bind(1, COMMITTED_TEST_ID);
	BOOST_REQUIRE(selectRes.next());
}

BOOST_FIXTURE_TEST_CASE( SQLiteAffectedRowsTest, TestTableBaseFixture )
{
	query << CLR << "INSERT INTO test (id) VALUES(1)";
	BOOST_REQUIRE(db.execute(query));
	BOOST_REQUIRE(db.affectedRows() == 1);
	
	query << CLR << "INSERT INTO test (id) VALUES(?)";
	auto res = db.createStatement(query);
	res.bind(1, 2);
	BOOST_REQUIRE(res.execute());
	res.reset();
	res.bind(1, 3);
	BOOST_REQUIRE(res.execute());
	BOOST_REQUIRE(res.affectedRows() == 1);
}


BOOST_AUTO_TEST_SUITE_END()
