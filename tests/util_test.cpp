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

BOOST_AUTO_TEST_SUITE_END()
