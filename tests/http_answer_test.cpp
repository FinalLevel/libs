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
	BString buf;
	HttpAnswer httpAnswer(buf, HTTP_OK_STATUS, "text/html", false, CACHE_PREVENTING_HEADERS);
	httpAnswer.setContentLength();
	BOOST_CHECK( strstr(buf.c_str(), "Content-Length: 0000000000\r\n\r\n") != NULL);
}

BOOST_AUTO_TEST_CASE( HttpAnswerSetContentLength )
{
	BOOST_CHECK_NO_THROW (	
		BString buf;
		HttpAnswer httpAnswer(buf, HTTP_OK_STATUS, "text/html", false, CACHE_PREVENTING_HEADERS);
		const std::string TEST_STRING("bla bla bla");
		buf << TEST_STRING;
		httpAnswer.setContentLength();
		BString requiredAnswer;
		requiredAnswer << "Content-Length: 00000000" << TEST_STRING.size() << "\r\n\r\n" << TEST_STRING.c_str();
		BOOST_CHECK( strstr(buf.c_str(), requiredAnswer.c_str()) != NULL);
	);
}

BOOST_AUTO_TEST_CASE( MimeTypeFromFileName )
{
	BOOST_CHECK(MimeType::getMimeTypeFromFileName("test.Jpg") == MimeType::E_JPEG);
	BOOST_CHECK(MimeType::getMimeTypeFromFileName("test.jPeG") == MimeType::E_JPEG);
	BOOST_CHECK(MimeType::getMimeTypeFromFileName("t.gif") == MimeType::E_GIF);
	BOOST_CHECK(MimeType::getMimeTypeFromFileName("t.png") == MimeType::E_PNG);
	BOOST_CHECK(MimeType::getMimeTypeFromFileName(".png") == MimeType::E_UNKNOWN);
	BOOST_CHECK(MimeType::getMimeTypeFromFileName("png") == MimeType::E_UNKNOWN);
}

BOOST_AUTO_TEST_CASE( MimeTypeFromFileNameFlac )
{
	BOOST_CHECK(MimeType::getMimeTypeFromFileName("t.flac") == MimeType::E_FLAC);
}

BOOST_AUTO_TEST_CASE( MimeTypeFromFileNameVorbis )
{
	BOOST_CHECK(MimeType::getMimeTypeFromFileName("t.ogg") == MimeType::E_VORBIS);
	BOOST_CHECK(MimeType::getMimeTypeFromFileName("test.oga") == MimeType::E_VORBIS);
}

BOOST_AUTO_TEST_CASE( MimeTypeFromFileNameM4A )
{
	BOOST_CHECK(MimeType::getMimeTypeFromFileName("t.m4a") == MimeType::E_M4A);
}


BOOST_AUTO_TEST_SUITE_END()

