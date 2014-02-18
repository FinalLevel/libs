#pragma once
#ifndef __FL_ACCEPT_THREAD_HPP
#define	__FL_ACCEPT_THREAD_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Connection accept classes
///////////////////////////////////////////////////////////////////////////////

#include "thread.hpp"
#include "event_thread.hpp"
#include "socket.hpp"

namespace fl {
	namespace events {
		using namespace fl::network;
		
		class AcceptThread : public fl::threads::Thread
		{
		public:
			AcceptThread(EPollWorkerGroup *workerGroup, Socket *listenTo,  WorkEventFactory *eventFactory);
		private:
			virtual void run();
			EPollWorkerGroup *_workerGroup;
			Socket *_listenTo;
			WorkEventFactory *_eventFactory;
		};
	};
};

#endif	// __FL_ACCEPT_THREAD_HPP
