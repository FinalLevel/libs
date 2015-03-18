///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Different text processing utility function unit tests
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>

#include "text_util.hpp"

using namespace fl::utils;
using fl::strings::BString;

BOOST_AUTO_TEST_SUITE( TextUtils )

BOOST_AUTO_TEST_CASE( stripHtmlTagsTest )
{
	BString simpleHtml { "<html><head><title>Bla bla</title></head><body>Test</body>" };
	stripHtmlTags(simpleHtml);
	BOOST_REQUIRE(simpleHtml == "Test");
}


BOOST_AUTO_TEST_CASE( stripHtmlTagsWihoutHTMLTest )
{
	BString simpleHtml { "Bla bla" };
	stripHtmlTags(simpleHtml);
	BOOST_REQUIRE(simpleHtml == "Bla bla");	
}

BOOST_AUTO_TEST_CASE( stripHtmlTagsBrTagTest )
{
	BString simpleHtml { "<br/>Bla<br/>bla" };
	stripHtmlTags(simpleHtml);
	BOOST_REQUIRE(simpleHtml == "Bla bla");	
}

BOOST_AUTO_TEST_CASE( stripHtmlTagsUnclosedTagTest )
{
	BString simpleHtml { "<html><head><tile>Bla bla</title><body>Test</body>" };
	stripHtmlTags(simpleHtml);
	BOOST_REQUIRE(simpleHtml == "");	
}

BOOST_AUTO_TEST_CASE( stripHtmlTagsAllowedTagTest )
{
	BString simpleHtml { "<a href='#'>Test</a>" };
	stripHtmlTags(simpleHtml,{"a"});
	BOOST_REQUIRE(simpleHtml == "<a href='#'>Test</a>");	
}


BOOST_AUTO_TEST_CASE( stripHtmlTagsDeniedContainerTest )
{
	BString simpleHtml { "<head><tile>Bla bla</title><script></head>test</script>" };
	stripHtmlTags(simpleHtml);
	BOOST_REQUIRE(simpleHtml == "test");	
}

BOOST_AUTO_TEST_CASE( stripHtmlTagsWithSourceBufferTest )
{
	BString result;
	std::string input { "<body>Test</body>" };
	std::string originInput = input;
	stripHtmlTags(input.c_str(), input.size(), result);
	BOOST_REQUIRE(result == "Test");	
	BOOST_REQUIRE(input == originInput);
}

BOOST_AUTO_TEST_SUITE_END()				