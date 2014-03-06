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

using namespace fl::network;

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



BOOST_AUTO_TEST_SUITE_END()
