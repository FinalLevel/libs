///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Program option class unit tests
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>

#include "program_option.hpp"

using namespace fl::utils;

BOOST_AUTO_TEST_SUITE( utils_ProgramOption )

BOOST_AUTO_TEST_CASE( ProgramOptionParse )
{
	const int argc = 7;
	const char *argv[argc] = {"test_program", "-c", "config", "-f",  "-d", "data", "-p"};
	ProgramOption options(argc, argv);
	BOOST_CHECK(options.options()[0].name == 'c' && options.options()[0].value == "config");
	BOOST_CHECK(options.options()[1].name == 'f' && options.options()[1].value.empty());
	BOOST_CHECK(options.options()[2].name == 'd' && options.options()[2].value == "data");
	BOOST_CHECK(options.options()[3].name == 'p' && options.options()[3].value.empty());
}

BOOST_AUTO_TEST_SUITE_END()