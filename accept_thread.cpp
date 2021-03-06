///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Connection accept classes
///////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include "accept_thread.hpp"
#include "log.hpp"
#include "socket.hpp"

using namespace fl::events;
using namespace fl::network;

AcceptThread::AcceptThread(EPollWorkerGroup *workerGroup, Socket *listenTo,  WorkEventFactory *eventFactory,
	uint32_t deferredAcceptTimeout, uint32_t defaultTimeout)
	: _workerGroup(workerGroup), _listenTo(listenTo), _eventFactory(eventFactory), _defaultTimeout(defaultTimeout)
{
	if (deferredAcceptTimeout) {
		_listenTo->setDeferAccept(deferredAcceptTimeout);
	}
	const size_t ACCEPT_THREAD_STACK_SIZE = 100 * 1024;
	setStackSize(ACCEPT_THREAD_STACK_SIZE);
	if (!create())
		throw exceptions::Error("Cannot create EPollWorkerThread thread");		
}

void AcceptThread::run()
{
	while (1) {
		TIPv4 ip;
		TEventDescriptor clientDescr = _listenTo->acceptDescriptor(ip);
		if (clientDescr == INVALID_SOCKET) {
			log::Error::L("AcceptThread: Connection accept error\n");
			continue;
		};
		if (!Socket::setNonBlockIO(clientDescr)) {
			log::Error::L("AcceptThread: cannot setNonBlockIO\n");
			close(clientDescr);
			continue;
		}
		WorkEvent *event = _eventFactory->create(clientDescr, ip, EPollWorkerGroup::curTime.unix() + _defaultTimeout, _listenTo);
		if (!_workerGroup->addConnection(event, _listenTo))	{
			log::Error::L("AcceptThread: Cannot add an event to the threads work group\n");
			delete event;
		}
	}
}
