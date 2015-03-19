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
using fl::strings::CLR;

BOOST_AUTO_TEST_SUITE( TextUtils )

BOOST_AUTO_TEST_CASE( base64DecodeTest )
{
	BString result;
	std::string input { "VGVzdCBjb2RlINC20LjQt9C90Yw=" };
	BOOST_REQUIRE(base64Decode(result, input.c_str(), input.size()));
	BOOST_REQUIRE(result == "Test code жизнь");
}

BOOST_AUTO_TEST_CASE( quotedPrintableDecodeTest )
{
	BString result;
	std::string input { "Test code =D0=B6=D0=B8=D0=B7=D0=BD=D1=8C" };
	quotedPrintableDecode(result, input.c_str(), input.size());
	BOOST_REQUIRE(result == "Test code жизнь");
}

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

BOOST_AUTO_TEST_CASE(decodeHtmlEntitiesWithNumbersTest)
{
	BString data;
	data << "x=&#" << static_cast<int>('A') << "; OK";
	decodeHtmlEntities(data);
	BOOST_REQUIRE(data == "x=A OK");	
	
	// hex entity test
	data.sprintfSet("x=&#x%x; OK", static_cast<int>('A'));
	decodeHtmlEntities(data);
	BOOST_REQUIRE(data == "x=A OK");
	
	// check non entity
	data.sprintfSet("#37895 NY NY");
	decodeHtmlEntities(data);
	BOOST_REQUIRE(data == "#37895 NY NY");
}

BOOST_AUTO_TEST_CASE(decodeHtmlEntitiesWithStringsTest)
{
	BString data;
	data << "Is&nbsp;it&nbsp;space?";
	decodeHtmlEntities(data);
	BOOST_REQUIRE(data == "Is\u00A0it\u00A0space?");	
	
	// check non entity
	data << CLR << "Is&nbsp it&unk;space?";
	decodeHtmlEntities(data);
	BOOST_REQUIRE(data == "Is&nbsp it&unk;space?");	
	
}

BOOST_AUTO_TEST_SUITE_END()				