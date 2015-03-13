///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Iconv wrapper unit tests
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include <string>

#include "iconv.hpp"

using namespace fl::iconv;
using fl::strings::BString;

BOOST_AUTO_TEST_SUITE( IconvTest )

BOOST_AUTO_TEST_CASE( IconvWindows1251ToUtf8Test )
{
	static const std::string SOURCE_STRING {"тестовый перевод"};
	BString resultWindows1251;
	BOOST_REQUIRE(convert(SOURCE_STRING.c_str(), SOURCE_STRING.size(), resultWindows1251, "utf-8", ECharset::WINDOWS1251));
	BOOST_REQUIRE(resultWindows1251.empty() == false);
	BString resultUtf8;
	BOOST_REQUIRE(convert(resultWindows1251.c_str(), resultWindows1251.size(), resultUtf8, "windows-1251", ECharset::UTF8));
	BOOST_REQUIRE(resultUtf8.empty() == false);
	BOOST_REQUIRE(resultUtf8 == resultUtf8);
}

BOOST_AUTO_TEST_SUITE_END()

