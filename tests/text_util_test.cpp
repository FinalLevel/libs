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
#include "iconv.hpp"

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

BOOST_AUTO_TEST_CASE( stripHtmlTagsTest1 )
{
	BString simpleHtml { "<html><head><title>Bla bla</title></head><body><blockquote "
			"<div><br>Test1</div> <span> Test2 <br></span> <div>  Test1 <br>+<br>"
			" Test2 </div></blockquote><div><p>Test<br ttt>test<br><br>end</div></body>" };
	stripHtmlTags(simpleHtml, {"br", "blockquote"});
	stripBlockquote(simpleHtml.c_str(), simpleHtml.size());
	replaceTags(simpleHtml, {{"br", {'\n'}}});
	BOOST_REQUIRE(simpleHtml == "\n>\n Test \ntest \n \nend");
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

BOOST_AUTO_TEST_CASE(decodeMimeHeaderWithNotMimeCharsTest)
{
	BString src;
	const std::string srcText("неправильный UTF8 mime");
	src << srcText;
	BString result;
	decodeMimeHeader(src, result, "UTF-8", NULL);
	BOOST_REQUIRE(result == src);
	
	src.clear();
	fl::iconv::convert(srcText.c_str(), srcText.size(), src, "utf-8", fl::iconv::ECharset::WINDOWS1251);
	decodeMimeHeader(src, result, "windows-1251", NULL);
	BOOST_REQUIRE(result == srcText);
}

BOOST_AUTO_TEST_CASE(decodeMimeHeaderWithQuotedPrintableTest)
{
	BString src;
	src << "=?utf-8?B?0JrQvtC90LrRg9GA0YEg0L7RgiDQn9C+0LrRg9C/0L7QvSE=?=";
	BString result;
	decodeMimeHeader(src, result, "", NULL);
	BOOST_REQUIRE(result == "Конкурс от Покупон!");
}

BOOST_AUTO_TEST_CASE(decodeMimeHeaderWithBase64Test)
{
	BString src;
	src << "=?utf-8?B?0L/RgNC+0YHRjNCx0LA=?=";
	BString result;
	decodeMimeHeader(src, result, "", NULL);
	BOOST_REQUIRE(result == "просьба");
}

BOOST_AUTO_TEST_CASE(decodeMimeHeaderEscapeTest)
{
	BString src;
	src << "\"=?UTF-8?B?4peEU3VwZXJEZWFsINCi0L7QstCw0YDRi+KWug==?=\" <support@superdeal.com.ua>";
	BString result;
	decodeMimeHeader(src, result, "", " \",;<\\");
	BOOST_REQUIRE(result == "\"◄SuperDeal\\ Товары►\" <support@superdeal.com.ua>");
}

BOOST_AUTO_TEST_CASE(StripReplyTest)
{
	BString buf;
	buf 	<< "post:"
			<< "\n"
			<< "SomeBody 11:04 wrote:"
			<< "\n"
			<< ">reply1"
			<< ">\r\n"
			<< "  inner post"
			<< "\r\n"
			<< ">>reply2"
			<< ">>\n";

	stripPreviewText(buf);
	BOOST_REQUIRE(buf == "post:\n  inner post");
}

BOOST_AUTO_TEST_CASE(StripReplyTest1)
{
	BString buf;
	buf 	<< "post"
			<< "\n"
			<< ">reply1"
			<< ">\r\n"
			<< " inner post"
			<< "\r\n"
			<< ">>reply2"
			<< ">>\n";

	stripPreviewText(buf);
	BOOST_REQUIRE(buf == "post\n inner post");
}

BOOST_AUTO_TEST_CASE(StripReplyTest2)
{
	BString buf;
	buf 	<< "On 05.15 15:44, SomeBody wrote:"
			<< "\n"
			<< ">reply1"
			<< ">\r\n"
			<< " inner post"
			<< "\r\n"
			<< ">>reply2"
			<< ">>\n"
			<< "post"
			<< "\n";

	stripPreviewText(buf);
	BOOST_REQUIRE(buf == "inner post\r\npost");
}

BOOST_AUTO_TEST_CASE(StripReplyTest3)
{
	BString buf;
	buf 	<< "\n"
			<< " "
			<< "post"
			<< "\n"
			<< "--"
			<< "\n"
			<< "signature"
			<< "\r\n"
			<< "SomeBody 11:04 wrote:"
			<< "\n";

	stripPreviewText(buf);
	BOOST_REQUIRE(buf == "post");
}

BOOST_AUTO_TEST_SUITE_END()				
