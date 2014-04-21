///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: HttpAnswer class unit tests
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>

#include "http_answer.hpp"

using namespace fl::http;

BOOST_AUTO_TEST_SUITE( fl_http_HttpAnswer )

BOOST_AUTO_TEST_CASE( HttpAnswerConstructor )
{
	BOOST_CHECK_NO_THROW (
		BString buf;
		HttpAnswer httpAnswer(buf, HTTP_OK_STATUS, "text/html", CACHE_PREVENTING_HEADERS);
		
	);
}

BOOST_AUTO_TEST_CASE( HttpAnswerSetContentLength )
{
	BOOST_CHECK_NO_THROW (	
		BString buf;
		HttpAnswer httpAnswer(buf, HTTP_OK_STATUS, "text/html", CACHE_PREVENTING_HEADERS);
		const std::string TEST_STRING("bla bla bla");
		buf << TEST_STRING;
		httpAnswer.setContentLength();
		BString requiredAnswer;
		requiredAnswer << "Content-Length: 00000000" << TEST_STRING.size() << "\r\n\r\n" << TEST_STRING.c_str();
		BOOST_CHECK( strstr(buf.c_str(), requiredAnswer.c_str()) != NULL);
	);
}

BOOST_AUTO_TEST_SUITE_END()

