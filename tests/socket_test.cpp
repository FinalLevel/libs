///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Socket class unit tests
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp> 

#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "socket.hpp"
#include "bstring.hpp"
#include "thread.hpp"

using namespace fl::network;
using namespace fl::threads;

BOOST_AUTO_TEST_SUITE( Network )

BOOST_AUTO_TEST_CASE( SocketCreate )
{
	BOOST_CHECK_NO_THROW (Socket socket; );
}

BOOST_AUTO_TEST_CASE( SocketListen )
{
	BOOST_CHECK_NO_THROW (
		Socket socket; 
		BOOST_CHECK(socket.listen("127.0.0.1", 43000) != false);
	);
}

BOOST_AUTO_TEST_CASE( SocketIp2String )
{
	BOOST_CHECK(Socket::ip2String(ntohl(inet_addr("192.110.71.3"))) == "192.110.71.3");
	BOOST_CHECK(Socket::ip2String(ntohl(inet_addr("123.234.223.101"))) == "123.234.223.101");
}

BOOST_AUTO_TEST_CASE( SocketResolve )
{
	BString buf;
	BOOST_CHECK(Socket::resolve("localhost", buf) == ntohl(inet_addr("127.0.0.1")));
}

const std::string TEST_DATA("12345678910");


class SendThread : public Thread
{
public:
	SendThread()
	{
		if (!create())
			throw std::exception();
	}
private:
	virtual void run()
	{
		Socket client;
		BOOST_CHECK(client.connect(Socket::ip2Long("127.0.0.1"), 43000) != false);
		char buf[TEST_DATA.size() + 1];
		BOOST_REQUIRE(client.pollAndSendAll(TEST_DATA.c_str(), TEST_DATA.size()));
		BOOST_REQUIRE(client.pollAndRecvAll(buf, TEST_DATA.size()));
		buf[TEST_DATA.size()] = 0;
		BOOST_CHECK(buf == TEST_DATA);
	};
};

class RecvThread : public Thread
{
public:
	RecvThread()
	{
		if (!create())
			throw std::exception();
	}
private:
	virtual void run()
	{
		Socket listen;
		BOOST_REQUIRE(listen.listen("127.0.0.1", 43000) != false);
		
		SendThread sendThread;
		
		TIPv4 ip;
		auto clientDescr = listen.acceptDescriptor(ip);
		BOOST_REQUIRE(clientDescr != INVALID_SOCKET);
		Socket client(clientDescr);
		char buf[TEST_DATA.size() + 1];
		BOOST_REQUIRE(client.pollAndRecvAll(buf, TEST_DATA.size()));
		buf[TEST_DATA.size()] = 0;
		BOOST_CHECK(buf == TEST_DATA);
		BOOST_REQUIRE(client.pollAndSendAll(buf, TEST_DATA.size()));
		sendThread.waitMe();
	};
};

BOOST_AUTO_TEST_CASE( connectAcceptRecvSend )
{
	BOOST_CHECK_NO_THROW (
		Socket socket; 
		BOOST_CHECK(socket.connect(Socket::ip2Long("127.0.0.1"), 43000, 200) == false);
	);
	try
	{
		RecvThread recvThread;
		recvThread.waitMe();
	}
	catch (...)
	{
		BOOST_CHECK_NO_THROW(throw);
	}	
}



BOOST_AUTO_TEST_SUITE_END()
