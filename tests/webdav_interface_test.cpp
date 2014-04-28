//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: WebDAV interface class unit tests
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include "mock_http_util.hpp"
#include "webdav_interface.hpp"
#include "socket.hpp"
#include "accept_thread.hpp"
#include "compatibility.hpp"

using namespace fl::network;
using namespace fl::events;
using namespace fl::http;



BOOST_AUTO_TEST_SUITE( WebDavInterafaceTest )

class MockPutWebDavInterface : public WebDavInterface
{
public:
	MockPutWebDavInterface()
	{
	}
	static std::string ANSWER;
	static std::string SMALL_FILE;
	typedef uint8_t TStatus;
	static const TStatus ST_PUT = 0x1;
	static const TStatus ST_FILE_PUT = 0x2;
	static bool checkStatus()
	{
		return _status == (ST_PUT | ST_FILE_PUT);
	}
protected:
	static TStatus _status;
	virtual EFormResult _formPut(BString &networkBuffer, class HttpEvent *http) override
	{
		auto result = WebDavInterface::_formPut(networkBuffer, http);
		if (WebDavInterface::_status & ST_POST_SPLITED) {
			_error = ERROR_400_BAD_REQUEST;
			if (!_postTmpFile.descr())
				return EFormResult::RESULT_ERROR;
			auto fileSize = _postTmpFile.fileSize();
			if (fileSize != (ssize_t)(_maxPostInMemmorySize + 1))
				return EFormResult::RESULT_ERROR;
			_postTmpFile.seek(0, SEEK_SET);
			BString data;
			char *buf = data.reserveBuffer(fileSize);
			if (_postTmpFile.read(buf, fileSize) != fileSize)
				return EFormResult::RESULT_ERROR;
			for (int i = 0; i < fileSize; i++)
				if (buf[i] != ('0' + (i % 32)))
					return EFormResult::RESULT_ERROR;
			_error = ERROR_200_OK;
			_status |= ST_FILE_PUT;
			return result;
	
		} else {
			if (_putData.c_str() == MockPutWebDavInterface::SMALL_FILE) {
				_status |= ST_PUT;
				return result;
			}
		}
		_error = ERROR_400_BAD_REQUEST;
		return EFormResult::RESULT_ERROR;
	}
};

std::string MockPutWebDavInterface::ANSWER("HTTP/1.1 201 Created\r\nContent-Type: text/xml; charset=\"utf-8\"\r\n\
Connection: Keep-Alive\r\nContent-Length: 0000000000\r\n\r\n");

std::string MockPutWebDavInterface::SMALL_FILE("Small test file\n");
MockPutWebDavInterface::TStatus MockPutWebDavInterface::_status = 0;

BOOST_AUTO_TEST_CASE( PutCheck )
{
	try
	{
		HttpMockEventFactory<MockPutWebDavInterface> factory;
		TestHttpEventFramework testEventFramework(&factory);
		BString request;
		request << "PUT /test HTTP/1.1\r\n";
		request << "Content-length: " << MockPutWebDavInterface::SMALL_FILE.size() << "\r\n";
		request << "\r\n" << MockPutWebDavInterface::SMALL_FILE; 
		
		Socket conn;
		BOOST_REQUIRE(testEventFramework.connect(conn));
		BString answer(MockPutWebDavInterface::ANSWER.size() + 1);
		BOOST_REQUIRE(testEventFramework.doRequest(conn, request, answer));
		BOOST_REQUIRE(answer == MockPutWebDavInterface::ANSWER.c_str());
		
		request.clear();
		BString testData;
		testData.clear();
		auto dataSize = WebDavInterface::maxPostInMemmorySize() + 1;
		auto pData = testData.reserveBuffer(dataSize);
		for (size_t i = 0; i < dataSize; i++) {
			pData[i] = '0' + (i % 32);
		}
		request << "PUT /test HTTP/1.1\r\n";
		request << "Content-length: " << testData.size() << "\r\n";
		request << "\r\n" << testData; 
		answer.clear();
		BOOST_REQUIRE(testEventFramework.doRequest(conn, request, answer));
		BOOST_REQUIRE(answer == MockPutWebDavInterface::ANSWER.c_str());
		BOOST_REQUIRE(MockPutWebDavInterface::checkStatus());
	}
	catch (...)
	{
		BOOST_CHECK_NO_THROW(throw);
	}
}

class MockMinimalWebDavInterface : public WebDavInterface
{
public:
	MockMinimalWebDavInterface()
	{
	}
	static std::string OPTIONS_ANSWER;
protected:
	virtual bool _put(const char *dataStart) 
	{
		_error = ERROR_400_BAD_REQUEST;
		return false;
	}
	virtual bool _putFile()
	{
		_error = ERROR_400_BAD_REQUEST;
		return false;
	}
};

std::string MockMinimalWebDavInterface::OPTIONS_ANSWER("HTTP/1.1 200 OK\r\nContent-Type: text/xml; charset=\"utf-8\"\r\n\
Connection: Keep-Alive\r\nContent-Length: 0000000000\r\n\
Allow: OPTIONS, GET, HEAD, POST, PUT, DELETE\r\n\
Allow: MKCOL, PROPFIND, PROPPATCH\r\n\
DAV: 1\r\n\r\n");


BOOST_AUTO_TEST_CASE( MinimalCheck )
{
	try
	{
		HttpMockEventFactory<MockMinimalWebDavInterface> factory;
		TestHttpEventFramework testEventFramework(&factory);
		BString request;
		request << "OPTIONS /test/ HTTP/1.1\r\n";
		request << "Content-length: " << 0 << "\r\n";
		request << "\r\n\r\n";
		
		Socket conn;
		BOOST_REQUIRE(testEventFramework.connect(conn));
		BString answer(MockMinimalWebDavInterface::OPTIONS_ANSWER.size() + 1);
		BOOST_REQUIRE(testEventFramework.doRequest(conn, request, answer));
		BOOST_REQUIRE(answer == MockMinimalWebDavInterface::OPTIONS_ANSWER.c_str());
		
		BString xmlData;
		xmlData << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\
    <propfind xmlns=\"DAV:\">\
      <prop>\
        <supported-method-set/>\
      </prop>\
    </propfind>";
		request.clear();
		request << "PROPFIND /test/ HTTP/1.1\r\n";
		request << "Depth: 0\r\nContent-Type: text/xml; charset=\"utf-8\"\r\nContent-length: " << xmlData.size() << "\r\n";
		request << "\r\n\r\n" << xmlData;
		
		answer.clear();
		BOOST_REQUIRE(testEventFramework.doRequest(conn, request, answer));
		BOOST_REQUIRE(strstr(answer.c_str(), "HTTP/1.1 207 Multi-Status\r\n") != NULL);	
	}
	catch (...)
	{
		BOOST_CHECK_NO_THROW(throw);
	}
}
	
BOOST_AUTO_TEST_SUITE_END()