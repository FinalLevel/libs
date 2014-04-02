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
#include "http_event.hpp"
#include "socket.hpp"
#include "accept_thread.hpp"

using namespace fl::network;
using namespace fl::events;


class MockThreadSpecificDataFactory : public ThreadSpecificDataFactory
{
public:
	MockThreadSpecificDataFactory()
	{
		
	}
	virtual ThreadSpecificData *create()
	{
		return new HttpThreadSpecificData();
	}
	virtual ~MockThreadSpecificDataFactory() {};
};


template <class T>
class HttpMockEventFactory : public WorkEventFactory 
{
public:
	HttpMockEventFactory()
	{
	}
	virtual WorkEvent *create(const TEventDescriptor descr, const TIPv4 ip, const time_t timeOutTime, 
		Socket *acceptSocket)
	{
		return new HttpEvent(descr, timeOutTime, new T());
	}
	virtual ~HttpMockEventFactory() {};
};
		
class TestHttpEventFramework
{
public:
	TestHttpEventFramework(WorkEventFactory *factory)
		: _ip("127.0.0.1"), _port(2000 + rand() % 10000), _acceptThread(NULL), _workerGroup(NULL)
	{		
		do {
			_port++;
		} while (!_listen.listen(_ip.c_str(), _port));
		_workerGroup = new EPollWorkerGroup(new MockThreadSpecificDataFactory(), 1, 10, 200000);
		_acceptThread = new AcceptThread(_workerGroup, &_listen, factory);
	};
	~TestHttpEventFramework()
	{
		_acceptThread->cancel();
		_acceptThread->waitMe();
		delete _acceptThread;
		delete _workerGroup;
	}
	bool doRequest(const BString &request, BString &answer)
	{
		Socket conn;
		if (!conn.connect(Socket::ip2Long(_ip.c_str()), _port))
			return false;
		if (!conn.pollAndSendAll(request.c_str(), request.size()))
			return false;
		size_t answerSize = answer.reserved() - 1;
		if (!conn.pollAndRecvAll(answer.reserveBuffer(answerSize), answerSize))
			return false;
		return true;
	}
private:
	Socket _listen;
	std::string _ip;
	uint16_t _port;
	AcceptThread *_acceptThread;
	EPollWorkerGroup *_workerGroup;
};

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
	virtual bool parseURI(const EHttpRequestType::EHttpRequestType reqType, const EHttpVersion::EHttpVersion version,
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
const std::string CreateDestructionMockHttpEventInterface::ANSWER("HTTP 200 OK\r\n\r\n");

BOOST_AUTO_TEST_CASE( CreateDestruction )
{
	try
	{
		HttpMockEventFactory<CreateDestructionMockHttpEventInterface> factory;
		TestHttpEventFramework testEventFramework(&factory);
		BString answer(CreateDestructionMockHttpEventInterface::ANSWER.size() + 1);
		BOOST_REQUIRE(testEventFramework.doRequest("GET / HTTP/1.0\r\n\r\n", answer));
		BOOST_CHECK(answer == CreateDestructionMockHttpEventInterface::ANSWER.c_str());
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
	}
	virtual bool parseURI(const EHttpRequestType::EHttpRequestType reqType, const EHttpVersion::EHttpVersion version,
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
const std::string FunctionalityMockHttpEventInterface::ANSWER("HTTP 200 OK\r\n\r\n");
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
	BOOST_CHECK(CreateDestructionMockHttpEventInterface::checkStatus());
}

BOOST_AUTO_TEST_SUITE_END()