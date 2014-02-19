///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: String class unit tests
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp> 

#include "bstring.hpp"

using namespace fl::strings;

BOOST_AUTO_TEST_SUITE( BStringTests )

BOOST_AUTO_TEST_CASE( Create )
{
	BOOST_CHECK_NO_THROW (
		BString str;
		BOOST_CHECK(str.size() == 0);
		BOOST_CHECK(str.reserved() == 0);
	);

	BOOST_CHECK_NO_THROW (
		BString str(10);
		BOOST_CHECK(str.size() == 0);
		BOOST_CHECK(str.reserved() == 10);
	);
}

BOOST_AUTO_TEST_CASE( MoveCreate )
{
	BOOST_CHECK_NO_THROW (
		BString str(10);
		BOOST_CHECK(str.sprintfSet("test") == 4);
		
		BString strTo(std::move(str));

		BOOST_CHECK(str.size() == 0);
		BOOST_CHECK(str.reserved() == 0);

		BOOST_CHECK(strTo.size() == 4);
		BOOST_CHECK(strTo.reserved() == 10);
		BOOST_CHECK(strTo == "test");
	);
}

BOOST_AUTO_TEST_CASE( MoveAssignment )
{
	BOOST_CHECK_NO_THROW (
		BString str(10);
		BOOST_CHECK(str.sprintfSet("test") == 4);
		
		BString strTo;
		strTo = std::move(str);

		BOOST_CHECK(str.size() == 0);
		BOOST_CHECK(str.reserved() == 0);

		BOOST_CHECK(strTo.size() == 4);
		BOOST_CHECK(strTo.reserved() == 10);
		BOOST_CHECK(strTo == "test");
	);
}

BOOST_AUTO_TEST_CASE( sprintfSet )
{
	BOOST_CHECK_NO_THROW (
		BString str;
		BOOST_CHECK(str.sprintfSet("1234%u", 5) == 5);
		BOOST_CHECK(str == "12345");
		BOOST_CHECK(str.size() == 5);
		BOOST_CHECK(str.reserved() == 6);
		
		BOOST_CHECK(str.sprintfSet("678%s", "9") == 4);
		BOOST_CHECK(str == "6789");
		BOOST_CHECK(str.size() == 4);
		BOOST_CHECK(str.reserved() == 6);
	);
}

BOOST_AUTO_TEST_CASE( sprintfAdd )
{
	BOOST_CHECK_NO_THROW (
		BString str;
		BOOST_CHECK(str.sprintfAdd("1234%u", 5) == 5);
		BOOST_CHECK(str == "12345");
		BOOST_CHECK(str.size() == 5);
		BOOST_CHECK(str.reserved() == 6);
		
		BOOST_CHECK(str.sprintfAdd("678%s", "9") == 4);
		BOOST_CHECK(str == "123456789");
		BOOST_CHECK(str.size() == 9);
		BOOST_CHECK(str.reserved() == 11);
	);
}

BOOST_AUTO_TEST_CASE( addString )
{
	BOOST_CHECK_NO_THROW (
		BString str;
		str.add("12345", strlen("12345"));
		BOOST_CHECK(str == "12345");
		BOOST_CHECK(str.size() == 5);
		BOOST_CHECK(str.reserved() == 6);
		
		str.add("6789", strlen("6789"));
		BOOST_CHECK(str == "123456789");
		BOOST_CHECK(str.size() == 9);
		BOOST_CHECK(str.reserved() == 10);
	);
}

BOOST_AUTO_TEST_CASE( AddNumber )
{
	BOOST_CHECK_NO_THROW (
		BString str;
		str << 12345;
		BOOST_CHECK(str == "12345");
		BOOST_CHECK(str.size() == 5);
		BOOST_CHECK(str.reserved() == 6);
		
		str << 6789;
		BOOST_CHECK(str == "123456789");
		BOOST_CHECK(str.size() == 9);
		BOOST_CHECK(str.reserved() == 11);
	
		str << 10 << 11;
		BOOST_CHECK(str == "1234567891011");
		BOOST_CHECK(str.size() == 13);
		BOOST_CHECK(str.reserved() == 14);
	);
}

BOOST_AUTO_TEST_CASE( AddChar )
{
	BOOST_CHECK_NO_THROW (
		BString str;
		str << 'a';
		BOOST_CHECK(str == "a");
		BOOST_CHECK(str.size() == 1);
		BOOST_CHECK(str.reserved() == 2);
		
		str << 'b' << 'c';
		BOOST_CHECK(str == "abc");
		BOOST_CHECK(str.size() == 3);
		BOOST_CHECK(str.reserved() == 4);
	);
}

BOOST_AUTO_TEST_CASE( clear )
{
	BOOST_CHECK_NO_THROW (
		BString str;
		str << "abc";
		BOOST_CHECK(str == "abc");
		str.clear();
		BOOST_CHECK(str.size() == 0);
		BOOST_CHECK(str.reserved() == 4);
	);
}

BOOST_AUTO_TEST_SUITE_END()
