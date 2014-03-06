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
#include "bstring.hpp"
#include <cinttypes>

namespace fl {
	namespace network {
		
		typedef int TDescriptor;
		typedef uint32_t TIPv4;
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
			Socket(const TDescriptor descr);
			~Socket();
			static const int MAX_LISTEN_BACKLOG = 512;
			bool listen(const char *listenIP, int port, const int maxListenBacklog = MAX_LISTEN_BACKLOG);
			
			TDescriptor acceptDescriptor(TIPv4 &ip);
			bool setDeferAccept(const int timeOut);
			void reset(const TDescriptor descr);
			const TDescriptor descr() const
			{
				return _descr;
			}
			static const size_t DEFAULT_READ_TIMEOUT = 60 * 1000; // 60 seconds
			bool pollAndRecvAll(void *buf, const size_t size, const size_t timeout = DEFAULT_READ_TIMEOUT);
			static fl::strings::BString ip2String(const TIPv4 ip);
			static bool setNonBlockIO(const TDescriptor descr);
		private:
			TDescriptor _descr;
		};
	}
};

#endif	// __FL_SOCKET_HPP
