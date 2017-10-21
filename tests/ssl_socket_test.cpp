///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: SSLSocket class unit tests
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ssl_socket.hpp"
#include "bstring.hpp"
#include "thread.hpp"

using namespace fl::network;
using namespace fl::threads;

BOOST_AUTO_TEST_SUITE( Network )

BOOST_AUTO_TEST_CASE( SSLSocketCreate )
{
	BOOST_CHECK_NO_THROW (SSLSocket socket; );
}


BOOST_AUTO_TEST_CASE( SSLSocketHTTPS )
{
  BString buf;
  SSLSocket socket;
  BOOST_REQUIRE(socket.connect("www.google.com", SSLSocket::HTTPS_PORT, buf) == true);
  buf.clear();
  buf << R"(GET / HTTP/1.0
Host: www.google.com

)";
  BOOST_REQUIRE(socket.pollAndSendAll(buf.c_str(), buf.size()) == true);
  buf.clear();
  const size_t readBufSize = 1024;
  auto readBuf = buf.reserveBuffer(readBufSize);
  auto res = socket.pollAndRecv(readBuf, readBufSize);
  BOOST_REQUIRE(res > 0);
  readBuf[res] = 0;
  BOOST_REQUIRE(strstr(readBuf, "HTTP") != nullptr);
}

BOOST_AUTO_TEST_SUITE_END()
