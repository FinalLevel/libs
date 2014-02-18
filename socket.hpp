#pragma once
#ifndef __FL_SOCKET_HPP
#define	__FL_SOCKET_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: socket wrapper class
///////////////////////////////////////////////////////////////////////////////

#include "exception.hpp"

namespace fl {
	namespace network {
		
		typedef int TDescriptor;
		const TDescriptor INVALID_SOCKET = -1;
		
		class NetworkError : public fl::exceptions::Error
		{
		public:
			NetworkError(const char *what)
				: fl::exceptions::Error(what)
			{
			}
		};
		
		class Socket 
		{
		public:
			Socket();
			~Socket();
			static const int MAX_LISTEN_BACKLOG = 512;
			bool listen(const char *listenIP, int port, const int maxListenBacklog = MAX_LISTEN_BACKLOG);
			
			TDescriptor acceptDescriptor();
			bool setDeferAccept(const int timeOut);
			static bool setNonBlockIO(const TDescriptor descr);
		private:
			TDescriptor _descr;
		};
	}
};

#endif	// __FL_SOCKET_HPP
