///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Time class unit tests
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>

#include "time.hpp"

using namespace fl::chrono;

BOOST_AUTO_TEST_SUITE( TimeUnitTests )

BOOST_AUTO_TEST_CASE( testParseHttpDate )
{
	std::string timeValue("Fri, 23 May 2014 13:13:33 GMT");
	auto res = Time::parseHttpDate(timeValue.c_str(), timeValue.size());
	BOOST_CHECK(res == 1400850813);
}

BOOST_AUTO_TEST_CASE( testTDayCreation )
{
	ETime eTime(1400850813);
	BOOST_REQUIRE(eTime.tDay() == 140523);
}


BOOST_AUTO_TEST_SUITE_END()
