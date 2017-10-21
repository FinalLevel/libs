#pragma once
#ifndef __FL_SSL_SOCKET_HPP
#define	__FL_SSL_SOCKET_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: OpenSSL socket wrapper class
///////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <mutex>
#include <openssl/ssl.h>
#include "socket.hpp"

namespace fl {
	namespace network {
    class SSLSocket {
    public:
      SSLSocket();
      SSLSocket(TDescriptor descr);
      ~SSLSocket();
      bool connect(const char *host, const uint32_t port, BString &buf,
        const size_t timeout = Socket::DEFAULT_CONNECT_TIMEOUT);
      bool pollAndSendAll(const void *buf, const size_t size,
        const size_t timeout = Socket::DEFAULT_SEND_TIMEOUT);
      ssize_t pollAndRecv(void *buf, const size_t size,
        const size_t timeout = Socket::DEFAULT_READ_TIMEOUT);
      static const uint32_t HTTPS_PORT = 443;

      void close();
      bool reopen();
    private:
      Socket _socket;
      SSL *_pSSL;
      bool _waitFor(const int sslErr, const int timeout);

      class OpenSSL {
      public:
        OpenSSL();
        // necessary functions to make OpenSSL thread safe
        static void lockCallBack(int mode, int type, const char *file, int line);
        static unsigned long currentThreadId(void);
        static std::vector<std::mutex> _locks;
      };
      static OpenSSL _openSSL;
    };
  }
}

#endif	// __FL_SSL_SOCKET_HPP
