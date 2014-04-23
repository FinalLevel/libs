///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Mysql classes unit tests
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp> 

#include "mysql.hpp"

using namespace fl::db;

BOOST_AUTO_TEST_SUITE( MysqlTest )

BOOST_AUTO_TEST_CASE( CreationTest )
{
	BOOST_CHECK_NO_THROW (
		Mysql sql;
	);	
}				

BOOST_AUTO_TEST_CASE( MakeQuery )
{
	BOOST_CHECK_NO_THROW (
		Mysql sql;
		auto query = sql.createQuery();
	);	
}

BOOST_AUTO_TEST_CASE( QueryTest )
{
	try 
	{
		Mysql sql;
		auto query = sql.createQuery();
		query << "SELECT id FROM " << "table" << " WHERE id=" << ESC << 10;
		BOOST_CHECK(query == "SELECT id FROM table WHERE id=10");

		query.clear();
		query << "SELECT id FROM " << "table" << " WHERE id=" << ESC << "test";
		BOOST_CHECK(query == "SELECT id FROM table WHERE id='test'");
		
		query.clear();
		query << "SELECT id FROM " << "table" << " WHERE id=" << ESC << "test" << " AND i=" << ESC << 10 << " AND 1";
		BOOST_CHECK(query == "SELECT id FROM table WHERE id='test' AND i=10 AND 1");

	}
	catch (...) 
	{
		BOOST_CHECK_NO_THROW(throw);
	}
}

BOOST_AUTO_TEST_SUITE_END()				
