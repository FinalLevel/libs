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
			static const uint32_t NO_DEFFER_ACCEPT = 0;
			static const uint32_t DEFAULT_DEFFER_ACCEPT = 15;
			static const uint32_t DEFAULT_ACCEPT_TIMEOUT = 15;
			AcceptThread(EPollWorkerGroup *workerGroup, Socket *listenTo,  WorkEventFactory *eventFactory, 
				uint32_t deferredAcceptTimeout = DEFAULT_DEFFER_ACCEPT, uint32_t defaultTimeout = DEFAULT_ACCEPT_TIMEOUT);
		private:
			virtual void run();
			EPollWorkerGroup *_workerGroup;
			Socket *_listenTo;
			WorkEventFactory *_eventFactory;
			uint32_t _defaultTimeout;
		};
	};
};

#endif	// __FL_ACCEPT_THREAD_HPP
