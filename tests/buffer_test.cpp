///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Buffer class unit tests
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp> 

#include "buffer.hpp"

using namespace fl::utils;

BOOST_AUTO_TEST_SUITE( BufferTest )

BOOST_AUTO_TEST_CASE( CreationTest )
{
	BOOST_CHECK_NO_THROW (
		Buffer buffer;
		BOOST_CHECK(buffer.writtenSize() == 0);
		BOOST_CHECK(buffer.isEnded());
		BOOST_CHECK(buffer.reserved() == 0);
		
		buffer.reserve(10);
		BOOST_CHECK(buffer.writtenSize() == 0);
		BOOST_CHECK(buffer.isEnded());
		BOOST_CHECK(buffer.reserved() == 10);
	);	
}

BOOST_AUTO_TEST_CASE( AddGetTest )
{
	BOOST_CHECK_NO_THROW (
		Buffer buffer;
		buffer.add<u_int32_t>(10);
		buffer.add<u_int32_t>(20);
		BOOST_CHECK(buffer.writtenSize() == sizeof(u_int32_t) * 2);
		BOOST_CHECK(buffer.reserved() == (sizeof(u_int32_t) + 1) * 2);
		u_int32_t value;
		buffer.get(value);
		BOOST_CHECK(value == 10);
		BOOST_CHECK(buffer.isEnded() == false);

		buffer.get(value);
		BOOST_CHECK(value == 20);
		BOOST_CHECK(buffer.isEnded() == true);
		
		BOOST_CHECK_THROW(buffer.get(value), Buffer::Error);
	);	
}

BOOST_AUTO_TEST_CASE( AddStringTest )
{
	const std::string testString1("testData");
	const std::string testString2("testData");
	BOOST_CHECK_NO_THROW (
		Buffer buffer;
		buffer.add(testString1);
		buffer.add(testString2);
		BOOST_CHECK(buffer.writtenSize() == testString1.size() + testString2.size() + sizeof(Buffer::TSize) * 2);
		std::string value;
		buffer.get(value);
		BOOST_CHECK(value == testString1);
		BOOST_CHECK(buffer.isEnded() == false);

		buffer.get(value);
		BOOST_CHECK(value == testString2);
		BOOST_CHECK(buffer.isEnded() == true);
	);	
}

BOOST_AUTO_TEST_CASE( AddStructTest )
{
	struct TestStruct
	{
		int param1;
		int param2;
	};
	const TestStruct ts1 = {1,2};
	const TestStruct ts2 = {3,4};
	BOOST_CHECK_NO_THROW (
		Buffer buffer;
		buffer.add(&ts1, sizeof(ts1));
		buffer.add(&ts2, sizeof(ts2));
		BOOST_CHECK(buffer.writtenSize() == sizeof(TestStruct) * 2);
		
		TestStruct value;
		buffer.get(&value, sizeof(value));
		BOOST_CHECK(memcmp(&value, &ts1, sizeof(value)) == 0);
		BOOST_CHECK(buffer.isEnded() == false);

		buffer.get(&value, sizeof(value));
		BOOST_CHECK(memcmp(&value, &ts2, sizeof(value)) == 0);
		BOOST_CHECK(buffer.isEnded() == true);
	);	
}

BOOST_AUTO_TEST_CASE( RewindClearTest )
{
	struct TestStruct
	{
		int param1;
		int param2;
	};
	const TestStruct ts1 = {1,2};
	const std::string testString = "testValue";
	BOOST_CHECK_NO_THROW (
		Buffer buffer;
		buffer.add(&ts1, sizeof(ts1));
		buffer.add(testString);
		
		TestStruct value;
		buffer.get(&value, sizeof(value));
		BOOST_CHECK(memcmp(&value, &ts1, sizeof(value)) == 0);
		BOOST_CHECK(buffer.isEnded() == false);

		std::string stringValue;
		buffer.get(stringValue);
		BOOST_CHECK(stringValue == testString);
		BOOST_CHECK(buffer.isEnded() == true);
		
		buffer.rewind();
		BOOST_CHECK(buffer.isEnded() == false);
		
		buffer.get(&value, sizeof(value));
		BOOST_CHECK(memcmp(&value, &ts1, sizeof(value)) == 0);

		buffer.get(stringValue);
		BOOST_CHECK(stringValue == testString);
		
		buffer.clear();
		buffer.add(testString);
		buffer.get(stringValue);
		BOOST_CHECK(stringValue == testString);
	);	
}

BOOST_AUTO_TEST_CASE( MoveFromBString )
{
	BOOST_CHECK_NO_THROW (
		BString bstr;
		bstr << "1234";
		Buffer::TSize size = bstr.size();
		Buffer::TSize resereved = bstr.reserved();
		Buffer buf(std::move(bstr));
		BOOST_CHECK(buf.writtenSize() == size);
		BOOST_CHECK(buf.reserved() == resereved);
		BOOST_CHECK(bstr.reserved() == 0);
	);
}

BOOST_AUTO_TEST_CASE( TruncateTest )
{
	BOOST_CHECK_NO_THROW (
		Buffer buffer;
		buffer.add<u_int32_t>(10);
		buffer.add<u_int32_t>(20);
		BOOST_CHECK(buffer.writtenSize() == sizeof(u_int32_t) * 2);
		BOOST_CHECK(buffer.reserved() == (sizeof(u_int32_t) + 1) * 2);
		
		buffer.truncate(buffer.writtenSize() -  sizeof(u_int32_t));
		
		BOOST_CHECK(buffer.writtenSize() == sizeof(u_int32_t));
		BOOST_CHECK(buffer.reserved() == (sizeof(u_int32_t) + 1) * 2);
		
		BOOST_CHECK_THROW(buffer.truncate(sizeof(u_int32_t) * 2), Buffer::Error);
		BOOST_CHECK_THROW(buffer.truncate(-1), Buffer::Error);
		
		buffer.truncate(buffer.writtenSize() -  sizeof(u_int32_t));
		BOOST_CHECK(buffer.writtenSize() == 0);
	);
}


BOOST_AUTO_TEST_SUITE_END()
