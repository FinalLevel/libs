///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Class implements buffered network functions.
///////////////////////////////////////////////////////////////////////////////

#include <cerrno>
#include <sys/socket.h>
#include "network_buffer.hpp"

using namespace fl::network;


NetworkBuffer::NetworkBuffer(NetworkBuffer &&moveFrom)
	: BString(std::move(moveFrom)), _sended(moveFrom._sended)
{
	moveFrom._sended = 0;
}

NetworkBuffer& NetworkBuffer::operator=(NetworkBuffer &&moveFrom)
{
	BString::operator=(std::move(moveFrom));
	std::swap(_sended, moveFrom._sended);
	return *this;
}

void NetworkBuffer::setSended(const TSize sended)
{
	if (sended > _size)
		throw BString::Error("Try to set sended out of size");
	_sended = sended;
}

NetworkBuffer::EResult NetworkBuffer::send(const TDescriptor descr)
{
	TSize leftSend = _size - _sended;
	if (leftSend <= 0)
		return OK;

	int res;
	while ((res = ::send(descr, _data + _sended, leftSend, MSG_NOSIGNAL)) > 0)	{
		_sended += res;
		if (res < leftSend)
			leftSend -= res;
		else
			return OK;
	}
	if (errno == EAGAIN || errno == EINTR)
		return IN_PROGRESS;
	else
		return ERROR;
}

NetworkBuffer::EResult NetworkBuffer::read(const TDescriptor descr)
{
	_sended = 0;
	int chunkSize = _reserved;
	if (_size) {
		chunkSize -= _size;
		if (chunkSize < (_reserved / 4))
			chunkSize = _reserved; // double buf after using of 1/4
	}
	chunkSize--;

	char *data = reserveBuffer(chunkSize);
	int res = recv(descr,  data,  chunkSize, MSG_NOSIGNAL | MSG_DONTWAIT);
	if (res > 0) {
		trim(_size - (chunkSize - res));
		return OK;
	}
	trim(_size - chunkSize);
	if (res == 0)
		return CONNECTION_CLOSE;

	if (errno == EAGAIN)
		return IN_PROGRESS;
	else
		return ERROR;
}

NetworkBufferPool::NetworkBufferPool(const int bufferSize, const uint32_t freeBuffersLimit)
	: _bufferSize(bufferSize), _freeBuffersLimit(freeBuffersLimit)
{
	
}

NetworkBuffer *NetworkBufferPool::get()
{
	if (_freeBuffers.empty())	{
		return new NetworkBuffer(_bufferSize);
	}	else {
		NetworkBuffer *buf = _freeBuffers.back();
		_freeBuffers.pop_back();
		return buf;
	}
}

void NetworkBufferPool::free(NetworkBuffer *buf)
{
	if (_freeBuffers.size() < _freeBuffersLimit) {
		buf->clear();
		if (buf->reserved() > _bufferSize)
			buf->reserve(_bufferSize);
		_freeBuffers.push_back(buf);
	} else {
		delete buf;
	}
}
