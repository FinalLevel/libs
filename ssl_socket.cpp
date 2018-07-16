///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: socket wrapper class
///////////////////////////////////////////////////////////////////////////////

#include <signal.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <openssl/err.h>
#include "ssl_socket.hpp"

using namespace fl::network;


std::vector<std::mutex> SSLSocket::OpenSSL::_locks;

void SSLSocket::OpenSSL::lockCallBack(int mode, int type, const char *file, int line) {
  if (mode & CRYPTO_LOCK) {
    _locks[type].lock();
  } else {
    _locks[type].unlock();
  }
}

unsigned long SSLSocket::OpenSSL::currentThreadId(void) {
  return (unsigned long) pthread_self();
}

SSLSocket::OpenSSL::OpenSSL() {
  /* Load encryption & hashing algorithms for the SSL program */
  SSL_library_init();

  /* Load the error strings for SSL & CRYPTO APIs */
  SSL_load_error_strings();

  /* Ignore Broken Pipe signal */
  signal(SIGPIPE, SIG_IGN);
  std::vector<std::mutex> locks(CRYPTO_num_locks());
  _locks.swap(locks);
  CRYPTO_set_id_callback(currentThreadId);
  CRYPTO_set_locking_callback(lockCallBack);
}

SSLSocket::OpenSSL SSLSocket::_openSSL;


SSLSocket::SSLSocket()
  : _pSSL(nullptr)
{
}

SSLSocket::SSLSocket(TDescriptor descr)
  : _socket(descr), _pSSL(nullptr)
{
  Socket::setNonBlockIO(descr);
}

SSLSocket::~SSLSocket() {
  close();
}

bool SSLSocket::_waitFor(const int sslErr, const int timeoutMs) {
  int events = 0;
	if (sslErr == SSL_ERROR_WANT_READ) {
		events |= POLLIN;
	} else if (sslErr == SSL_ERROR_WANT_WRITE) {
		events |= POLLOUT;
	} else {
		return false;
	}
  struct pollfd pollList[1];
  pollList[0].fd = _socket.descr();
  pollList[0].events = events;
  pollList[0].revents = 0;

  int retval = poll(pollList, 1, timeoutMs);
  if(retval < 0) {
    return false;
  }
  if (retval == 0) {
    return false;
  }

  if (((pollList[0].revents & POLLHUP) == POLLHUP)
    || ((pollList[0].revents & POLLERR) == POLLERR)
    || ((pollList[0].revents & POLLNVAL) == POLLNVAL)
  ) {
    return false;
  }
  if (events == POLLIN) {
    if ((pollList[0].revents & POLLIN) != POLLIN)
      return false;
  }
  if (events == POLLOUT) {
    if ((pollList[0].revents & POLLOUT) != POLLOUT)
      return false;
  }
  return true;
}

bool SSLSocket::connect(const char *host, const uint32_t port,
  BString &buf, const size_t timeout) {
  if (!_socket.connect(host, port, buf, timeout)) {
    return false;
  }
  const SSL_METHOD *meth = SSLv23_client_method();
  /* Create a SSL_CTX structure */
  SSL_CTX *ctx = SSL_CTX_new(meth);
  if (ctx == nullptr) {
    return false;
  }
  close();
  _pSSL = SSL_new(ctx);

  SSL_CTX_free(ctx);

  if (_pSSL == nullptr) {
    return false;
  }
  SSL_set_tlsext_host_name(_pSSL, host);

  /* Assign the socket into the SSL structure (SSL and socket without BIO) */
  if (SSL_set_fd(_pSSL, _socket.descr()) != 1) {
    return false;
  }

  /* Perform SSL Handshake */
  int ret;
  while ((ret = SSL_connect(_pSSL)) != 1) {
    int err = SSL_get_error(_pSSL, ret);
    if (!_waitFor(err, timeout)) {
      auto sslerr = ERR_get_error();
      char buf[130];
      ERR_error_string(sslerr, buf);
      printf("%s\n", buf);
      return false;
    }
  }
  return true;
}

bool SSLSocket::pollAndSendAll(const void *buf, const size_t size, const size_t timeout)
{
	size_t leftSize = size;
	size_t sended = 0;
	while (leftSize > 0) {
		auto res = SSL_write(_pSSL, static_cast<const uint8_t*>(buf) + sended, leftSize);
		if (res < 0) {
      int err = SSL_get_error(_pSSL, res);
      if (!_waitFor(err, timeout)) {
        break;
      }
      continue;
		} else if (res == 0) {
			return false;
    }
		sended += res;
		leftSize -= res;
	}
	return true;
}

ssize_t SSLSocket::pollAndRecv(void *buf, const size_t size, const size_t timeout)
{
  int res;
  while ((res = SSL_read(_pSSL, buf, size)) <= 0) {
    int err = SSL_get_error(_pSSL, res);
    if (!_waitFor(err, timeout))
      break;
  }
  return res;
}

void SSLSocket::close()	{
  if (_pSSL != nullptr) {
		size_t retry = 6;
		while (SSL_shutdown(_pSSL) == 0) {
			if (retry-- <= 0) {
				break;
			}
		}
		SSL_free(_pSSL);
		_pSSL = nullptr;
	}
}

bool SSLSocket::reopen() {
  close();
  return _socket.reopen();
}
