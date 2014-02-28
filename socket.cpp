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

#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#include "socket.hpp"

using namespace fl::network;

Socket::Socket()
{
	if ((_descr = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)	{
		throw NetworkError("Cannot create socket");
	}
}

Socket::~Socket()
{
	if (_descr != INVALID_SOCKET)
	{
		close(_descr);
	}
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

TDescriptor Socket::acceptDescriptor()
{
	struct	sockaddr_in	sock_addr;
	socklen_t	sa_size = sizeof(sock_addr);
	bzero(&sock_addr, sa_size);

	TDescriptor clientDescr = accept(_descr, (struct sockaddr *)&sock_addr, &sa_size);
	if (clientDescr == INVALID_SOCKET) 	{
		return INVALID_SOCKET;
	}
	else {
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
