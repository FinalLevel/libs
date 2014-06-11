///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Different utilities functions
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp> 

#include "util.hpp"

using namespace fl::utils;

BOOST_AUTO_TEST_SUITE( Utils )

BOOST_AUTO_TEST_CASE( testConvertStringTo )
{
	BOOST_CHECK(convertStringTo<u_int32_t>("12", NULL, 10) == 12);
	BOOST_CHECK(convertStringTo<u_int32_t>("12") == 12);
	
	char *pstr;
	BOOST_CHECK(convertStringTo<u_int64_t>("1F1F1F1F1F1Fz", &pstr, 16) == 0x1F1F1F1F1F1Fll);
	BOOST_CHECK(*pstr == 'z');
	
	BOOST_CHECK(convertStringTo<int>("-12") == -12);
}

BOOST_AUTO_TEST_CASE( testFileExists )
{
	BOOST_CHECK(fileExists("/"));
}

BOOST_AUTO_TEST_CASE( testExplode )
{
	const char *str = "1,2,3,4,5";
	std::vector<int> ids;
	BOOST_CHECK( explode<int>(str, ids) );
	BOOST_CHECK( ids.size() == 5 );
	auto id = ids.begin();
	for (int i = 1; i <= 5; i++) {
		BOOST_CHECK(*id == i);
		id++;
	}
}

BOOST_AUTO_TEST_CASE( testParseSizeString )
{
	BOOST_CHECK( parseSizeString("0k")  == 0);
	BOOST_CHECK( parseSizeString("10")  == 10);
	BOOST_CHECK( parseSizeString("10k")  == (10 * 1024));
	BOOST_CHECK( parseSizeString("10M")  == (10 * 1024 * 1024));
	BOOST_CHECK( parseSizeString("1g")  == (1 * 1024 * 1024 * 1024));
}

BOOST_AUTO_TEST_CASE( testHex2Binary )
{
	const std::string hex("d950b8ccbb5be478");
	uint64_t binary;
	hex2BinaryBackOrder(hex.c_str(), (uint8_t*)&binary, sizeof(binary));
	BOOST_CHECK( binary == strtoull(hex.c_str(), NULL, 16));
	
	const std::string bigHex("0102030405060708090a0B0C0D0E0F1011121314");
	uint8_t bigBinary[20];
	hex2BinaryBackOrder(bigHex.c_str(), bigBinary, sizeof(bigBinary));
	for (uint8_t i = 0; i < sizeof(bigBinary); i++) {
		BOOST_CHECK( bigBinary[ i ] == sizeof(bigBinary) - i );
	}
	
}

BOOST_AUTO_TEST_SUITE_END()
