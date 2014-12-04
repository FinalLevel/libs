///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Http event system classes unit tests
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include "mock_http_util.hpp"
#include "compatibility.hpp"

using namespace fl::network;
using namespace fl::events;



BOOST_AUTO_TEST_SUITE( HttpEventTest )

class CreateDestructionMockHttpEventInterface : public HttpEventInterface
{
public:
	typedef uint32_t TStatus;
	static const TStatus ST_CREATE = 0x1;
	static const TStatus ST_DESTROY = 0x2;
	static TStatus _status;
	CreateDestructionMockHttpEventInterface()
	{
		_status |= ST_CREATE;
	}
	virtual bool parseURI(const char *cmdStart, const EHttpVersion::EHttpVersion version,
			const std::string &host, const std::string &fileName, const std::string &query)
	{
		return true;
	}
	static const std::string ANSWER;
	virtual EFormResult formResult(BString &networkBuffer, class HttpEvent *http)
	{
		networkBuffer << ANSWER;
		return RESULT_OK_CLOSE;
	}
	virtual ~CreateDestructionMockHttpEventInterface()
	{
		_status |= ST_DESTROY;
	}
	static bool checkStatus()
	{
		return _status == (ST_CREATE | ST_DESTROY);
	}
};

CreateDestructionMockHttpEventInterface::TStatus CreateDestructionMockHttpEventInterface::_status = 0;
const std::string CreateDestructionMockHttpEventInterface::ANSWER("HTTP/1.0 200 OK\r\n\r\n");

BOOST_AUTO_TEST_CASE( CreateDestruction )
{
	try
	{
		HttpMockEventFactory<CreateDestructionMockHttpEventInterface> factory;
		TestHttpEventFramework testEventFramework(&factory);
		BString answer(CreateDestructionMockHttpEventInterface::ANSWER.size() + 1);
		BOOST_REQUIRE(testEventFramework.doRequest("GET / HTTP/1.0\r\n\r\n", answer));
		BOOST_CHECK(answer == CreateDestructionMockHttpEventInterface::ANSWER.c_str());
		for (int i = 0; i < 10; i++) {
			if (CreateDestructionMockHttpEventInterface::checkStatus())
				break;
			else {
				struct timespec tim;
				tim.tv_sec = 0;
				tim.tv_nsec = 20000;
				nanosleep(&tim , NULL);
			}
		}
	}
	catch (...)
	{
		BOOST_CHECK_NO_THROW(throw);
	}
	BOOST_CHECK(CreateDestructionMockHttpEventInterface::checkStatus());
}

class FunctionalityMockHttpEventInterface : public HttpEventInterface
{
public:
	typedef uint32_t TStatus;
	static TStatus _status;
	static const TStatus ST_COOKIE = 0x1;
	static const TStatus ST_URI = 0x2;
	FunctionalityMockHttpEventInterface()
	{
		_status = 0;
	}
	virtual bool parseURI(const char *cmdStart, const EHttpVersion::EHttpVersion version,
			const std::string &host, const std::string &fileName, const std::string &query)
	{
		if (fileName == TEST_FILE_NAME) {
			if (query.empty())
				return true;
			const char *paramStart = query.c_str();
			const char *pEnd = query.c_str() + query.size();
			const char *value;
			size_t valueLength;
			char param;
			int a = 0;
			std::string b;
			while ((param = _nextParam(paramStart, pEnd, value, valueLength)) != 0)
			{
				switch (param)
				{
				case 'a':
					a = atoi(value);
				break;
				case 'b':
					b.assign(value, valueLength);
				break;
				};
			}
			if ((a == 1) && (b == "test1"))
				_status |= ST_URI;
			return true;
		}
		else
			return false;
	}
	
	static const std::string ANSWER;
	virtual EFormResult formResult(BString &networkBuffer, class HttpEvent *http)
	{
		networkBuffer << ANSWER;
		return RESULT_OK_CLOSE;
	}
	virtual ~FunctionalityMockHttpEventInterface()
	{
	}
	virtual bool parseHeader(const char *name, const size_t nameLength, const char *value, const size_t valueLen, 
				const char *pEndHeader)
	{
		if (_isCookieHeader(name, nameLength)) {
			if (valueLen != TEST_COOKIE.size())
				return false;
			if (!strncmp(value, TEST_COOKIE.c_str(), TEST_COOKIE.size())) {
				_status |= ST_COOKIE;
			}
			else
				return false;
		}
		return true;
	}
	static const std::string TEST_COOKIE;
	static const std::string TEST_FILE_NAME;
	static const std::string TEST_QUERY;
};

FunctionalityMockHttpEventInterface::TStatus FunctionalityMockHttpEventInterface::_status = 0;
const std::string FunctionalityMockHttpEventInterface::ANSWER("HTTP/1.0 200 OK\r\n\r\n");
const std::string FunctionalityMockHttpEventInterface::TEST_COOKIE("U=test");
const std::string FunctionalityMockHttpEventInterface::TEST_FILE_NAME("/test");
const std::string FunctionalityMockHttpEventInterface::TEST_QUERY("a1&btest1");

BOOST_AUTO_TEST_CASE( FunctionalityTest )
{
	try
	{
		HttpMockEventFactory<FunctionalityMockHttpEventInterface> factory;
		TestHttpEventFramework testEventFramework(&factory);
		BString answer(FunctionalityMockHttpEventInterface::ANSWER.size() + 1);
		BString request;
		request << "GET " << FunctionalityMockHttpEventInterface::TEST_FILE_NAME << '?' 
							<<  FunctionalityMockHttpEventInterface::TEST_QUERY<< " HTTP/1.0\r\n";
		request << "Cookie: " << FunctionalityMockHttpEventInterface::TEST_COOKIE << "\r\n";
		request << "\r\n"; 
		BOOST_REQUIRE(testEventFramework.doRequest(request, answer));
		BOOST_CHECK(answer == FunctionalityMockHttpEventInterface::ANSWER.c_str());
		BOOST_CHECK(FunctionalityMockHttpEventInterface::_status & FunctionalityMockHttpEventInterface::ST_COOKIE);
		BOOST_CHECK(FunctionalityMockHttpEventInterface::_status & FunctionalityMockHttpEventInterface::ST_URI);
	}
	catch (...)
	{
		BOOST_CHECK_NO_THROW(throw);
	}
}

class KeepAliveMockHttpEventInterface : public HttpEventInterface
{
public:
	typedef uint32_t TStatus;
	static TStatus _status;
	static const TStatus ST_COOKIE1 = 0x1;
	static const TStatus ST_COOKIE2 = 0x2;
	static const TStatus ST_URI1 = 0x4;
	static const TStatus ST_URI2 = 0x8;
	static const TStatus ST_KEEP_ALIVE = 0x10;
	
	KeepAliveMockHttpEventInterface()
		: _isKeepAlive(true)
	{
		_status = 0;
		reset();
	}
	std::string _fileName;
	bool _isKeepAlive;
	
	virtual bool reset()
	{
		if (_isKeepAlive) {
			_fileName.clear();
			_isKeepAlive = false;
			return true;
		} else 
			return false;
	};
	
	virtual bool parseURI(const char *cmdStart, const EHttpVersion::EHttpVersion version,
			const std::string &host, const std::string &fileName, const std::string &query)
	{
		if (version == EHttpVersion::HTTP_1_1) {
			_status |= ST_KEEP_ALIVE;
			_isKeepAlive = true;
		}
		_fileName = fileName;
		if (query.empty())
			return true;
		const char *paramStart = query.c_str();
		const char *pEnd = query.c_str() + query.size();
		const char *value;
		size_t valueLength;
		char param;
		int a = 0;
		std::string b;
		while ((param = _nextParam(paramStart, pEnd, value, valueLength)) != 0) {
			switch (param)
			{
			case 'a':
				a = atoi(value);
			break;
			case 'b':
				b.assign(value, valueLength);
			break;
			};
		}
		if (fileName == TEST_FILE_NAME1) {
			if ((a == 1) && (b == "test1"))
				_status |= ST_URI1;
		} else if (fileName == TEST_FILE_NAME2) {
			if ((a == 2) && (b == "test2"))
				_status |= ST_URI2;		
		}
		return true;
	}
	
	static const std::string ANSWER1;
	static const std::string ANSWER2;
	virtual EFormResult formResult(BString &networkBuffer, class HttpEvent *http)
	{
		if (_fileName == TEST_FILE_NAME1)
			networkBuffer << ANSWER1;
		else if (_fileName == TEST_FILE_NAME2)
			networkBuffer << ANSWER2;
		else
			return RESULT_ERROR;
		if (_isKeepAlive)
			return RESULT_OK_KEEP_ALIVE;
		else
			return RESULT_OK_CLOSE;
	}
	virtual ~KeepAliveMockHttpEventInterface()
	{
	}
	virtual bool parseHeader(const char *name, const size_t nameLength, const char *value, const size_t valueLen, 
				const char *pEndHeader)
	{
		bool isKeepAlive = false;
		if (_parseKeepAlive(name, nameLength, value, isKeepAlive)) {
			if (isKeepAlive) {
				_status |= ST_KEEP_ALIVE;
				_isKeepAlive = true;
			} else {
				_status &= ~ST_KEEP_ALIVE;
				_isKeepAlive = false;				
			}
		} else if (_isCookieHeader(name, nameLength)) {
			if (_fileName == TEST_FILE_NAME1) { 
				if (valueLen != TEST_COOKIE1.size())
					return false;
				if (!strncmp(value, TEST_COOKIE1.c_str(), TEST_COOKIE1.size())) {
					_status |= ST_COOKIE1;
				}
			} else if (_fileName == TEST_FILE_NAME2) {
				if (valueLen != TEST_COOKIE2.size())
					return false;
				if (!strncmp(value, TEST_COOKIE2.c_str(), TEST_COOKIE2.size())) {
					_status |= ST_COOKIE2;
				}
			}
			else
				return false;
		}
		return true;
	}
	static const std::string TEST_COOKIE1;
	static const std::string TEST_FILE_NAME1;
	static const std::string TEST_COOKIE2;
	static const std::string TEST_FILE_NAME2;
	static const std::string TEST_QUERY1;
	static const std::string TEST_QUERY2;
};

KeepAliveMockHttpEventInterface::TStatus KeepAliveMockHttpEventInterface::_status = 0;
const std::string KeepAliveMockHttpEventInterface::ANSWER1("HTTP/1.1 200 OK\r\nContent-length: 5\r\n\r\ntest1");
const std::string KeepAliveMockHttpEventInterface::ANSWER2("HTTP/1.1 200 OK\r\nContent-length: 5\r\n\r\ntest2");

const std::string KeepAliveMockHttpEventInterface::TEST_COOKIE1("U=test");
const std::string KeepAliveMockHttpEventInterface::TEST_FILE_NAME1("/test");
const std::string KeepAliveMockHttpEventInterface::TEST_COOKIE2("K=00332221HA");
const std::string KeepAliveMockHttpEventInterface::TEST_FILE_NAME2("/file/file2");
const std::string KeepAliveMockHttpEventInterface::TEST_QUERY1("a1&btest1");
const std::string KeepAliveMockHttpEventInterface::TEST_QUERY2("a2&btest2");

BOOST_AUTO_TEST_CASE( KeepAliveTest )
{
	try
	{
		HttpMockEventFactory<KeepAliveMockHttpEventInterface> factory;
		TestHttpEventFramework testEventFramework(&factory);
		BString answer1(KeepAliveMockHttpEventInterface::ANSWER1.size() + 1);
		BString request1;
		request1 << "GET " << KeepAliveMockHttpEventInterface::TEST_FILE_NAME1 << '?' 
							<<  KeepAliveMockHttpEventInterface::TEST_QUERY1 << " HTTP/1.0\r\n";
		request1 << "Cookie: " << KeepAliveMockHttpEventInterface::TEST_COOKIE1 << "\r\n";
		request1 << "\r\n"; 
		BOOST_REQUIRE(testEventFramework.doRequest(request1, answer1));
		BOOST_CHECK(answer1 == KeepAliveMockHttpEventInterface::ANSWER1.c_str());
		BOOST_CHECK(KeepAliveMockHttpEventInterface::_status & KeepAliveMockHttpEventInterface::ST_COOKIE1);
		BOOST_CHECK(KeepAliveMockHttpEventInterface::_status & KeepAliveMockHttpEventInterface::ST_URI1);
		BOOST_CHECK(!(KeepAliveMockHttpEventInterface::_status & KeepAliveMockHttpEventInterface::ST_KEEP_ALIVE));
		
		request1.trim(request1.size() - 2);
		request1 << "Connection:  keep-Alive\r\n\r\n";
		answer1.clear();
		BOOST_REQUIRE(testEventFramework.doRequest(request1, answer1));
		BOOST_CHECK(answer1 == KeepAliveMockHttpEventInterface::ANSWER1.c_str());
		BOOST_CHECK(KeepAliveMockHttpEventInterface::_status & KeepAliveMockHttpEventInterface::ST_COOKIE1);
		BOOST_CHECK(KeepAliveMockHttpEventInterface::_status & KeepAliveMockHttpEventInterface::ST_URI1);
		BOOST_CHECK(KeepAliveMockHttpEventInterface::_status & KeepAliveMockHttpEventInterface::ST_KEEP_ALIVE);
		
		request1.clear();
		answer1.clear();
		request1 << "GET / HTTP/1.1\r\n\r\n";
		BOOST_REQUIRE(testEventFramework.doRequest(request1, answer1));
		BOOST_CHECK(KeepAliveMockHttpEventInterface::_status & KeepAliveMockHttpEventInterface::ST_KEEP_ALIVE);
		
		request1.clear();
		answer1.clear();
		request1 << "GET / HTTP/1.1\r\nConnection: close\r\n\r\n";
		BOOST_REQUIRE(testEventFramework.doRequest(request1, answer1));
		BOOST_CHECK(!(KeepAliveMockHttpEventInterface::_status & KeepAliveMockHttpEventInterface::ST_KEEP_ALIVE));
		
		
		Socket conn;
		BOOST_REQUIRE(testEventFramework.connect(conn));
		request1.clear();
		answer1.clear();
		request1 << "GET " << KeepAliveMockHttpEventInterface::TEST_FILE_NAME1 << '?' 
							<<  KeepAliveMockHttpEventInterface::TEST_QUERY1 << " HTTP/1.1\r\n";
		request1 << "Cookie: " << KeepAliveMockHttpEventInterface::TEST_COOKIE1 << "\r\n";
		request1 << "\r\n";
		BOOST_REQUIRE(testEventFramework.doRequest(conn, request1, answer1));
		BOOST_CHECK(answer1 == KeepAliveMockHttpEventInterface::ANSWER1.c_str());
		BOOST_CHECK(KeepAliveMockHttpEventInterface::_status & KeepAliveMockHttpEventInterface::ST_COOKIE1);
		BOOST_CHECK(KeepAliveMockHttpEventInterface::_status & KeepAliveMockHttpEventInterface::ST_URI1);
		BOOST_CHECK(KeepAliveMockHttpEventInterface::_status & KeepAliveMockHttpEventInterface::ST_KEEP_ALIVE);
		
		BString answer2(KeepAliveMockHttpEventInterface::ANSWER2.size() + 1);
		BString request2;
		request2 << "GET " << KeepAliveMockHttpEventInterface::TEST_FILE_NAME2 << '?' 
						 <<  KeepAliveMockHttpEventInterface::TEST_QUERY2 << " HTTP/1.1\r\nConnection: close\r\n";
		request2 << "Cookie: " << KeepAliveMockHttpEventInterface::TEST_COOKIE2 << "\r\n";
		request2 << "\r\n";
		BOOST_REQUIRE(testEventFramework.doRequest(conn, request2, answer2));
		BOOST_CHECK(answer2 == KeepAliveMockHttpEventInterface::ANSWER2.c_str());
		BOOST_CHECK(KeepAliveMockHttpEventInterface::_status & KeepAliveMockHttpEventInterface::ST_COOKIE1);
		BOOST_CHECK(KeepAliveMockHttpEventInterface::_status & KeepAliveMockHttpEventInterface::ST_URI1);	
		BOOST_CHECK(KeepAliveMockHttpEventInterface::_status & KeepAliveMockHttpEventInterface::ST_COOKIE2);
		BOOST_CHECK(KeepAliveMockHttpEventInterface::_status & KeepAliveMockHttpEventInterface::ST_URI2);
		BOOST_CHECK(!(KeepAliveMockHttpEventInterface::_status & KeepAliveMockHttpEventInterface::ST_KEEP_ALIVE));					
	}
	catch (...)
	{
		BOOST_CHECK_NO_THROW(throw);
	}
}

class PostMockHttpEventInterface : public HttpEventInterface
{
public:
	typedef uint32_t TStatus;
	static TStatus _status;
	static const TStatus ST_COOKIE = 0x1;
	static const TStatus ST_URI = 0x2;
	static const TStatus ST_POST = 0x4;
	static const TStatus ST_CONTENT_LENGTH = 0x8;
	
	int _a;
	std::string _b;
	size_t _contentLength;
	PostMockHttpEventInterface()
		: _a(0), _contentLength(0)
	{
		_status = 0;
	}
	bool _parseQuery(const std::string &query)
	{
		if (query.empty())
			return true;
		const char *paramStart = query.c_str();
		const char *pEnd = query.c_str() + query.size();
		const char *value;
		size_t valueLength;
		char param;
		while ((param = _nextParam(paramStart, pEnd, value, valueLength)) != 0)
		{
			switch (param)
			{
			case 'a':
				_a = atoi(value);
			break;
			case 'b':
				_b.assign(value, valueLength);
			break;
			};
		}
		if ((_a == 1) && (_b == "post1234"))
			_status |= ST_URI;
		return true;	
	}
	
	virtual bool parseURI(const char *cmdStart, const EHttpVersion::EHttpVersion version,
			const std::string &host, const std::string &fileName, const std::string &query)
	{
		auto reqType = _parseHTTPCmd(*cmdStart);
		if (reqType == EHttpRequestType::POST)
			_status |= ST_POST;
		if (fileName == TEST_FILE_NAME) {
			_parseQuery(query);
			return true;
		}
		else
			return false;
	}
	
	virtual bool parsePOSTData(const uint32_t postStartPosition, NetworkBuffer &buf, bool &parseError)
	{
		parseError = false;
		if (postStartPosition + _contentLength <= (size_t)buf.size()) {
			parseError = _parseQuery(std::string(buf.c_str() + postStartPosition, _contentLength));
			return true;
		}
		else
			return false;
	}
				
	
	static const std::string ANSWER;
	virtual EFormResult formResult(BString &networkBuffer, class HttpEvent *http)
	{
		networkBuffer << ANSWER;
		return RESULT_OK_CLOSE;
	}
	virtual ~PostMockHttpEventInterface()
	{
	}
	virtual bool parseHeader(const char *name, const size_t nameLength, const char *value, const size_t valueLen, 
				const char *pEndHeader)
	{
		if (_parseContentLength(name, nameLength, value, _contentLength)) {
			if (_contentLength == POST_QUERY.size())
				_status |= ST_CONTENT_LENGTH;
		} else if (_isCookieHeader(name, nameLength)) {
			if (valueLen != TEST_COOKIE.size())
				return false;
			if (!strncmp(value, TEST_COOKIE.c_str(), TEST_COOKIE.size())) {
				_status |= ST_COOKIE;
			}
			else
				return false;
		}
		return true;
	}
	static const std::string TEST_COOKIE;
	static const std::string TEST_FILE_NAME;
	static const std::string TEST_QUERY;
	static const std::string POST_QUERY;
};

PostMockHttpEventInterface::TStatus PostMockHttpEventInterface::_status = 0;
const std::string PostMockHttpEventInterface::ANSWER("HTTP/1.0 200 OK\r\n\r\n");
const std::string PostMockHttpEventInterface::TEST_COOKIE("U=test");
const std::string PostMockHttpEventInterface::TEST_FILE_NAME("/test");
const std::string PostMockHttpEventInterface::TEST_QUERY("a1&ktest1");
const std::string PostMockHttpEventInterface::POST_QUERY("a1&bpost1234&cblablabla");

BOOST_AUTO_TEST_CASE( HttpPostTest )
{
	try
	{
		HttpMockEventFactory<PostMockHttpEventInterface> factory;
		TestHttpEventFramework testEventFramework(&factory);
		BString answer(PostMockHttpEventInterface::ANSWER.size() + 1);
		BString request;
		request << "POST " << PostMockHttpEventInterface::TEST_FILE_NAME << '?' 
							<<  PostMockHttpEventInterface::TEST_QUERY<< " HTTP/1.0\r\n";
		request << "Cookie: " << PostMockHttpEventInterface::TEST_COOKIE << "\r\n";
		request << "Content-Length: " << PostMockHttpEventInterface::POST_QUERY.size() << "\r\n";
		request << "\r\n" << PostMockHttpEventInterface::POST_QUERY;
		BOOST_REQUIRE(testEventFramework.doRequest(request, answer));
		BOOST_CHECK(answer == PostMockHttpEventInterface::ANSWER.c_str());
		BOOST_CHECK(PostMockHttpEventInterface::_status & PostMockHttpEventInterface::ST_COOKIE);
		BOOST_CHECK(PostMockHttpEventInterface::_status & PostMockHttpEventInterface::ST_URI);
		BOOST_CHECK(PostMockHttpEventInterface::_status & PostMockHttpEventInterface::ST_CONTENT_LENGTH);
		BOOST_CHECK(PostMockHttpEventInterface::_status & PostMockHttpEventInterface::ST_POST);
		request.clear();
		answer.clear();
		const uint32_t BIG_POST_SIZE = 128000;
		request << "POST " << PostMockHttpEventInterface::TEST_FILE_NAME << '?' 
							<<  PostMockHttpEventInterface::TEST_QUERY<< " HTTP/1.0\r\n";
		request << "Cookie: " << PostMockHttpEventInterface::TEST_COOKIE << "\r\n";
		request << "Content-Length: " << (PostMockHttpEventInterface::POST_QUERY.size() + BIG_POST_SIZE) << "\r\n";
		request << "\r\n";
		request.reserve(request.reserved() + BIG_POST_SIZE + PostMockHttpEventInterface::POST_QUERY.size() + 1);
		for (uint32_t i = 0; i < BIG_POST_SIZE; i++)
			request << 'x';
		request << PostMockHttpEventInterface::POST_QUERY;
		
		BOOST_REQUIRE(testEventFramework.doRequest(request, answer));
		BOOST_CHECK(answer == PostMockHttpEventInterface::ANSWER.c_str());
		BOOST_CHECK(PostMockHttpEventInterface::_status & PostMockHttpEventInterface::ST_COOKIE);
		BOOST_CHECK(PostMockHttpEventInterface::_status & PostMockHttpEventInterface::ST_URI);
		BOOST_CHECK(PostMockHttpEventInterface::_status & PostMockHttpEventInterface::ST_POST);
	}
	catch (...)
	{
		BOOST_CHECK_NO_THROW(throw);
	}
}


class DependedMockHttpEventInterface : public HttpEventInterface
{
public:
	typedef uint32_t TStatus;
	static const TStatus ST_CREATE = 0x1;
	static const TStatus ST_DESTROY = 0x2;
	static const TStatus ST_DEPENDED_CREATE = 0x4;
	static const TStatus ST_DEPENDED_DESTROY = 0x8;
	
	static TStatus _status;
	HttpEvent *_http;
	DependedMockHttpEventInterface()
		: _http(NULL), _depended(this)
	{
		_status |= ST_CREATE;
	}
	virtual bool parseURI(const char *cmdStart, const EHttpVersion::EHttpVersion version,
			const std::string &host, const std::string &fileName, const std::string &query)
	{
		return true;
	}
	static const std::string ANSWER;
	virtual EFormResult formResult(BString &networkBuffer, class HttpEvent *http)
	{
		_http = http;
		_http->thread()->ctrl(&_depended);
		return RESULT_OK_WAIT;
	}
	virtual ~DependedMockHttpEventInterface()
	{
		_status |= ST_DESTROY;
	}
	static bool checkStatus()
	{
		return _status == (ST_CREATE | ST_DESTROY | ST_DEPENDED_CREATE | ST_DEPENDED_DESTROY);
	}
	void timerCall()
	{
		BString *networkBuffer = _http->networkBuffer();
		if (networkBuffer) {
			networkBuffer->clear();
			*networkBuffer << ANSWER;
			_http->sendAnswer(RESULT_OK_CLOSE);
		}
	}
	
	class DependedEvent : public TimerEvent
	{
	public:
		DependedMockHttpEventInterface *_parent;
		static const int long CALL_AFTER = 100000000; // 100 ms
		DependedEvent(DependedMockHttpEventInterface *parent)
			: TimerEvent(), _parent(parent) // call after CALL_AFTER nanosecond
		{
			if (!setTimer(0, CALL_AFTER, 0, 0))
				throw std::exception();
			DependedMockHttpEventInterface::_status |= ST_DEPENDED_CREATE;
		}
			
		virtual ~DependedEvent()
		{
			DependedMockHttpEventInterface::_status |= ST_DEPENDED_DESTROY;
		}
		
		virtual const ECallResult call(const TEvents events)
		{
			_parent->timerCall();
			return _readTimer();
		}
	};
	DependedEvent _depended;
};

DependedMockHttpEventInterface::TStatus DependedMockHttpEventInterface::_status = 0;
const std::string DependedMockHttpEventInterface::ANSWER("HTTP/1.0 200 OK\r\n\r\n");


BOOST_AUTO_TEST_CASE( Depended )
{
	try
	{
		HttpMockEventFactory<DependedMockHttpEventInterface> factory;
		TestHttpEventFramework testEventFramework(&factory);
		BString answer(DependedMockHttpEventInterface::ANSWER.size() + 1);
		BOOST_REQUIRE(testEventFramework.doRequest("GET / HTTP/1.0\r\n\r\n", answer));
		BOOST_CHECK(answer == DependedMockHttpEventInterface::ANSWER.c_str());
	}
	catch (...)
	{
		BOOST_CHECK_NO_THROW(throw);
	}
	BOOST_CHECK(DependedMockHttpEventInterface::checkStatus());
}

class PartialMockHttpEventInterface : public HttpEventInterface
{
public:
	typedef uint32_t TStatus;
	static const TStatus ST_CREATE = 0x1;
	static const TStatus ST_DESTROY = 0x2;
	static const TStatus ST_FORM_CALLED = 0x4;
	static const TStatus ST_GET_MORE_DATA_CALLED = 0x8;
	
	static TStatus _status;
	PartialMockHttpEventInterface()
	{
		_status |= ST_CREATE;
	}
	virtual bool parseURI(const char *cmdStart, const EHttpVersion::EHttpVersion version,
			const std::string &host, const std::string &fileName, const std::string &query)
	{
		return true;
	}
	static const std::string ANSWER_PART1;
	virtual EFormResult formResult(BString &networkBuffer, class HttpEvent *http) override
	{
		_status |= ST_FORM_CALLED;
		networkBuffer << ANSWER_PART1;
		return RESULT_OK_PARTIAL_SEND;
	}
	static const std::string ANSWER_PART2;
	virtual EFormResult getMoreDataToSend(BString &networkBuffer, class HttpEvent *http) override
	{
		_status |= ST_GET_MORE_DATA_CALLED;
		networkBuffer << ANSWER_PART2;
		return RESULT_OK_CLOSE;
	}
	virtual ~PartialMockHttpEventInterface()
	{
		_status |= ST_DESTROY;
	}
	static bool checkStatus()
	{
		return _status == (ST_CREATE | ST_DESTROY | ST_FORM_CALLED | ST_GET_MORE_DATA_CALLED);
	}	
};

PartialMockHttpEventInterface::TStatus PartialMockHttpEventInterface::_status = 0;
const std::string PartialMockHttpEventInterface::ANSWER_PART1("HTTP/1.0 200 OK\r\n\r\n");
const std::string PartialMockHttpEventInterface::ANSWER_PART2("Next data bla bla");


BOOST_AUTO_TEST_CASE( PartialSend )
{
	try
	{
		HttpMockEventFactory<PartialMockHttpEventInterface> factory;
		TestHttpEventFramework testEventFramework(&factory);
		BString answer(PartialMockHttpEventInterface::ANSWER_PART1.size() 
			+ PartialMockHttpEventInterface::ANSWER_PART2.size()  + 1);
		BOOST_REQUIRE(testEventFramework.doRequest("GET / HTTP/1.0\r\n\r\n", answer));
		BString resultAnswer;
		resultAnswer << PartialMockHttpEventInterface::ANSWER_PART1 << PartialMockHttpEventInterface::ANSWER_PART2;
		BOOST_CHECK(answer == resultAnswer);
	}
	catch (...)
	{
		BOOST_CHECK_NO_THROW(throw);
	}
	BOOST_CHECK(PartialMockHttpEventInterface::checkStatus());
}

class HttpRangesInterface : public HttpEventInterface
{
public:
	void testRanges()
	{
		const std::string RANGE_NAME{"Range"};
		int32_t rangeStart {0};
		uint32_t rangeEnd {0};
		
		const std::string fullRange {"bytes=1-9"};	
		BOOST_REQUIRE(_parseRange(RANGE_NAME.c_str(), RANGE_NAME.size(), fullRange.c_str(), fullRange.size(),
			rangeStart, rangeEnd));
		BOOST_REQUIRE(rangeStart == 1);
		BOOST_REQUIRE(rangeEnd == 9);
		
		const std::string startRange {"bytes=100-"};	
		BOOST_REQUIRE(_parseRange(RANGE_NAME.c_str(), RANGE_NAME.size(), startRange.c_str(), startRange.size(),
			rangeStart, rangeEnd));
		BOOST_REQUIRE(rangeStart == 100);
		BOOST_REQUIRE(rangeEnd == 0);
		
		const std::string endRange {"bytes=-200"};	
		BOOST_REQUIRE(_parseRange(RANGE_NAME.c_str(), RANGE_NAME.size(), endRange.c_str(), endRange.size(),
			rangeStart, rangeEnd));
		BOOST_REQUIRE(rangeStart == -200);
		BOOST_REQUIRE(rangeEnd == 0);
		
		rangeStart = rangeEnd = 100;
		const std::string badRange {"bytes=200-100"};	
		BOOST_REQUIRE(_parseRange(RANGE_NAME.c_str(), RANGE_NAME.size(), badRange.c_str(), badRange.size(),
			rangeStart, rangeEnd));
		BOOST_REQUIRE(rangeStart == 0);
		BOOST_REQUIRE(rangeEnd == 0);
	}
	bool parseURI(const char*, fl::events::EHttpVersion::EHttpVersion, const std::string&, const std::string&, 
		const std::string&) 
	{
		return false;
	}
	virtual EFormResult formResult(fl::strings::BString&, fl::events::HttpEvent*)
	{
		return RESULT_ERROR;
	}
};

BOOST_AUTO_TEST_CASE( HttpRanges )
{
	HttpRangesInterface http;
	http.testRanges();
}

BOOST_AUTO_TEST_SUITE_END()