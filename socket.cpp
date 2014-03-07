///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: socket wrapper class
///////////////////////////////////////////////////////////////////////////////

#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#include "socket.hpp"

using namespace fl::network;
using fl::strings::BString;

Socket::Socket()
{
	if (!_open())
		throw NetworkError("Cannot create socket");
}

bool Socket::_open()
{
	if ((_descr = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)	{
		return setNonBlockIO(_descr);
	} else {
		return true;
	}
}

bool Socket::reopen()
{
	reset(INVALID_SOCKET);
	return _open();
}

Socket::Socket(const TDescriptor descr)
	: _descr(descr)
{
}

Socket::~Socket()
{
	reset(INVALID_SOCKET);
}

void Socket::reset(const TDescriptor descr)
{
	if (_descr != INVALID_SOCKET)
		close(_descr);
	_descr = descr;
}

bool Socket::setDeferAccept(const int timeOut)
{
	int val = timeOut;
	if (setsockopt( _descr, SOL_TCP, TCP_DEFER_ACCEPT, (char *)&val, sizeof(val) ))	{
		return false;
	}
	else
		return true;
}

bool Socket::setNonBlockIO(const TDescriptor descr)
{
	int flags = fcntl(descr, F_GETFL, 0);
	if (fcntl(descr, F_SETFL, flags | O_NONBLOCK))
		return false;
	else
		return true;
}

TDescriptor Socket::acceptDescriptor(TIPv4 &ip)
{
	struct	sockaddr_in	sock_addr;
	socklen_t	sa_size = sizeof(sock_addr);
	bzero(&sock_addr, sa_size);

	TDescriptor clientDescr = accept(_descr, (struct sockaddr *)&sock_addr, &sa_size);
	if (clientDescr == INVALID_SOCKET) 	{
		return INVALID_SOCKET;
	}
	else {
		ip  = ntohl(sock_addr.sin_addr.s_addr);
		return clientDescr;
	}
}

bool Socket::listen(const char *listenIP, int port, const int maxListenBacklog)
{
	sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(listenIP);
	addr.sin_port = htons(port);


	int opt = 1;
	if (setsockopt(_descr, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)))
		return false;

	if (bind(_descr, (struct sockaddr *)&addr, sizeof(addr)))
		return false;

	if (::listen(_descr, maxListenBacklog))
		return false;
	
	return true;
}

bool Socket::connect(const TIPv4 ip, const uint32_t port, const size_t timeout)
{
	sockaddr_in addr;
	bzero(&addr, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = ntohl(ip);
	addr.sin_port = htons(port);
	struct pollfd socketList[1];
	int res;
	while ((res = ::connect(_descr, (sockaddr *)&addr, sizeof(addr))) != 0)
	{
		if (errno == EISCONN)
			return true;
		
		if ((errno != EINPROGRESS) && (errno != EAGAIN) && (errno != ENOTCONN))
			return false;

		socketList[0].fd = _descr;
		socketList[0].events = POLLOUT | POLLERR;
		socketList[0].revents = 0;
		res = poll(socketList, 1, timeout);
		if (res <= 0)
			return false;
		if(((socketList[0].revents & POLLHUP) == POLLHUP) ||
      ((socketList[0].revents & POLLERR) == POLLERR) ||
      ((socketList[0].revents & POLLNVAL) == POLLNVAL)) {
			return false;
    }
	}
	return true;
}

bool Socket::pollAndRecvAll(void *buf, const size_t size, const size_t timeout)
{
	struct pollfd socketList[1];
	size_t leftSize = size;
	size_t readed = 0;
	while (leftSize > 0) {
		socketList[0].fd = _descr;
		socketList[0].events = POLLIN | POLLERR | POLLHUP;
		socketList[0].revents = 0;
		auto retval = poll(socketList, 1, timeout);
		if (retval <= 0)
			return false;
		if(((socketList[0].revents & POLLHUP) == POLLHUP) ||
      ((socketList[0].revents & POLLERR) == POLLERR) ||
      ((socketList[0].revents & POLLNVAL) == POLLNVAL)) {
			return false;
    }
		auto res = recv(_descr, static_cast<uint8_t*>(buf) + readed, leftSize, MSG_NOSIGNAL | MSG_DONTWAIT);
		if (res < 0)
		{
			if (errno == EAGAIN)
				continue;
			return false;
		}
		else if (res == 0)
			return false;
		readed += res;
		leftSize -= res;
	}
	return true;
}

bool Socket::pollAndSendAll(const void *buf, const size_t size, const size_t timeout)
{
	struct pollfd socketList[1];
	size_t leftSize = size;
	size_t sended = 0;
	while (leftSize > 0) {
		socketList[0].fd = _descr;
		socketList[0].events = POLLOUT | POLLERR | POLLHUP;
		socketList[0].revents = 0;
		auto retval = poll(socketList, 1, timeout);
		if (retval <= 0)
			return false;
		if(((socketList[0].revents & POLLHUP) == POLLHUP) ||
      ((socketList[0].revents & POLLERR) == POLLERR) ||
      ((socketList[0].revents & POLLNVAL) == POLLNVAL)) {
			return false;
    }
		auto res = send(_descr, static_cast<const uint8_t*>(buf) + sended, leftSize, MSG_NOSIGNAL);
		if (res < 0)
		{
			if (errno == EAGAIN)
				continue;
			return false;
		}
		else if (res == 0)
			return false;
		sended += res;
		leftSize -= res;
	}
	return true;	
}

TIPv4 Socket::ip2Long(const char *ipStr)
{
	return inet_network(ipStr);
}

BString Socket::ip2String(const TIPv4 ip)
{
	static const int MAX_IP_LENGTH = 4 * 4;
	BString ipStr(MAX_IP_LENGTH + 1);
	
	static const char DECIMALS[] = "0123456789";
	char *curBuf = ipStr.reserveBuffer(MAX_IP_LENGTH);
	char *start = curBuf;
	for (int i = (sizeof(TIPv4) - 1) * 8; i >= 0; i -= 8)
	{
		uint8_t val = (ip >> i) & 0xFF;
		if (val >= 100)
		{
			if (val >= 200)
				*curBuf++ = '2';
			else
				*curBuf++ = '1';
			val %= 100;
			if (val < 10)
				*curBuf++ = '0';
		}
		if (val >= 10)
			*curBuf++ = DECIMALS[val / 10];
		*curBuf++ = DECIMALS[val % 10];
		*curBuf++ = '.';
	}
	ipStr.trim(curBuf - start - 1);
	return std::move(ipStr);
}
