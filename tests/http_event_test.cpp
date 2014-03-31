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
	virtual bool parseURI(const EHttpRequestType::EHttpRequestType reqType, const EHttpVersion::EHttpVersion version,
			const std::string &host, const std::string &fileName, const std::string &query)
	{
		return true;
	}
	virtual EFormResult formResult(BString &networkBuffer, class HttpEvent *http)
	{
		return RESULT_OK_CLOSE;
	}
};
	
BOOST_AUTO_TEST_CASE( CreateDestruction )
{
	HttpMockEventFactory<CreateDestructionMockHttpEventInterface> factory;
	TestHttpEventFramework testEventFramework(&factory);
}

BOOST_AUTO_TEST_SUITE_END()