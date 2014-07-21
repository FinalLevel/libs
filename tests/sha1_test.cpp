///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: SHA1 wrapper classes unit tests
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp> 

#include "sha1.hpp"
#include "util.hpp"

using namespace fl::crypto;

BOOST_AUTO_TEST_SUITE( FLSHA1 )

BOOST_AUTO_TEST_CASE( testSHA1HolderCreate )
{
	BOOST_CHECK_THROW(SHA1Holder("AAAA00", sizeof("AAAA00")), SHA1Exeption);
	BOOST_CHECK_THROW(SHA1Holder((SHA1Holder::TBinaryPtr)("AAAA00"), sizeof("AAAA00")), SHA1Exeption);
	try 
	{
		const std::string HEX_SHA1 = "D950B8CCBB5BE47815B10293FAF5A9ACDAE9E821";
		SHA1Holder sha1FromHex(HEX_SHA1.c_str(), HEX_SHA1.size());
		uint8_t binary[SHA1_BINARY_SIZE];
		fl::utils::hex2Binary(HEX_SHA1.c_str(), binary, SHA1_BINARY_SIZE);
		SHA1Holder sha1FromBinary(binary, SHA1_BINARY_SIZE);
		BOOST_CHECK(sha1FromHex == sha1FromBinary);
		BString backConv;
		sha1FromBinary.toBString(backConv);
		BOOST_CHECK(backConv == HEX_SHA1);

		SHA1Holder sha1Zero;
		SHA1Holder sha1ZeroFromHex("0000000000000000000000000000000000000000", SHA1_HEX_SIZE);
		BOOST_CHECK( sha1Zero == sha1ZeroFromHex );
	}
	catch (...) 
	{
		BOOST_CHECK_NO_THROW(throw);
	}
}

BOOST_AUTO_TEST_CASE( testSHA1Calculation )
{
	fl::utils::Buffer buf;
	std::string test("test string");
	buf.add(test.c_str(), test.size());
	SHA1Holder sha1Calc(buf);
	SHA1Holder sha1Hex("661295c9cbf9d6b2f6428414504a8deed3020641", SHA1_HEX_SIZE);
	
	BOOST_CHECK( sha1Calc == sha1Hex );
}

BOOST_AUTO_TEST_CASE( testLowerUpper )
{
	const std::string HEX_SHA1_UPPER = "D950B8CCBB5BE47815B10293FAF5A9ACDAE9E821";
	SHA1Holder sha1Upper(HEX_SHA1_UPPER.c_str(), HEX_SHA1_UPPER.size());
	
	const std::string HEX_SHA1_LOWER = "d950b8ccbb5be47815b10293faf5a9acdae9e821";
	SHA1Holder sha1Lower(HEX_SHA1_LOWER.c_str(), HEX_SHA1_LOWER.size());
	
	BOOST_CHECK( sha1Upper == sha1Lower );
}

BOOST_AUTO_TEST_CASE( testSHA1Builder )
{
	std::string TEST_DATA("blabla bla bla bla");
	SHA1Builder builder;
	
	fl::utils::Buffer buf;
	buf.add(TEST_DATA.c_str(), TEST_DATA.size());
	SHA1Holder sha1Calc(buf);
	for (int i = 0; i < 1; i++) {
		size_t half = TEST_DATA.size() / 2;
		builder.update(TEST_DATA.c_str(), half);
		builder.update(TEST_DATA.c_str() + half, TEST_DATA.size() - half);
		SHA1Holder buildedSHA1;
		builder.finish(buildedSHA1);
		
		BOOST_CHECK( sha1Calc == buildedSHA1 );
	}
}

BOOST_AUTO_TEST_CASE( testSHA1HolderFromFile )
{
	fl::fs::File fl;
	BOOST_REQUIRE(fl.createUnlinkedTmpFile("/tmp"));
	std::string TEST_DATA("bladsbla bla bla bla");
	BOOST_REQUIRE(fl.write(TEST_DATA.c_str(), TEST_DATA.size()) == (ssize_t)TEST_DATA.size());
	fl::utils::BString buffer(TEST_DATA.size() / 2);
	fl.seek(0, SEEK_SET);
	SHA1Holder sha1FromFile(fl, fl.fileSize(), buffer);
	
	fl::utils::Buffer buf;
	buf.add(TEST_DATA.c_str(), TEST_DATA.size());
	SHA1Holder sha1Calc(buf);
	BOOST_CHECK( sha1Calc == sha1FromFile );
}

BOOST_AUTO_TEST_SUITE_END()

