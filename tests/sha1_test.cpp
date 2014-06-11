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

BOOST_AUTO_TEST_SUITE( SHA1 )

BOOST_AUTO_TEST_CASE( testSHA1HolderCreate )
{
	BOOST_CHECK_THROW(SHA1Holder("AAAA00", sizeof("AAAA00")), SHA1Exeption);
	BOOST_CHECK_THROW(SHA1Holder((const uint8_t*)("AAAA00"), sizeof("AAAA00")), SHA1Exeption);
	try 
	{
		const char * const HEX_SHA1 = "d950b8ccbb5be47815b10293faf5a9acdae9e821";
		SHA1Holder sha1FromHex(HEX_SHA1, SHA1_HEX_SIZE);
		uint8_t binary[SHA1_BINARY_SIZE];
		fl::utils::hex2Binary(HEX_SHA1, binary, SHA1_BINARY_SIZE);
		SHA1Holder sha1FromBinary(binary, SHA1_BINARY_SIZE);
		BOOST_CHECK(sha1FromHex == sha1FromBinary);
		
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

BOOST_AUTO_TEST_SUITE_END()

