///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: URandom class unit tests
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>

#include "urandom.hpp"

using namespace fl::utils;

BOOST_AUTO_TEST_SUITE( UrandomUnitTests )

BOOST_AUTO_TEST_CASE( testUrandom )
{
	URandom urand;
	uint64_t val;
	BOOST_REQUIRE(urand.getBlock(&val, sizeof(val)));
	BOOST_REQUIRE(val != 0);
}

BOOST_AUTO_TEST_SUITE_END()