#pragma once
#ifndef __FL_MOCK_HTTP_UTIL_HPP
#define	__FL_MOCK_HTTP_UTIL_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Helper mock http event classes 
///////////////////////////////////////////////////////////////////////////////

#include "http_event.hpp"
#include "socket.hpp"
#include "accept_thread.hpp"


namespace fl {
	namespace events {
		using namespace fl::network;
		
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
				if (!connect(conn))
					return false;
				return doRequest(conn, request, answer);
			}
			bool doRequest(Socket &conn, const BString &request, BString &answer)
			{
				if (!conn.pollAndSendAll(request.c_str(), request.size()))
					return false;
				if (!conn.pollReadHttpAnswer(answer))
					return false;
				return true;
			}
			bool connect(Socket &conn)
			{
				if (!conn.connect(Socket::ip2Long(_ip.c_str()), _port))
					return false;
				else
					return true;
			}
		private:
			Socket _listen;
			std::string _ip;
			uint16_t _port;
			AcceptThread *_acceptThread;
			EPollWorkerGroup *_workerGroup;
		};
	};
};

#endif	// __FL_MOCK_HTTP_UTIL_HPP
