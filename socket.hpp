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
		typedef uint16_t TPort16;
		const TDescriptor INVALID_SOCKET = -1;
		using fl::strings::BString;
		
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
			
			Socket(const Socket &) = delete;
			Socket &operator=(const Socket &) = delete;
			
			Socket(Socket &&sock);
			Socket &operator=(Socket &&sock);
			
			static const int MAX_LISTEN_BACKLOG = 512;
			bool listen(const char *listenIP, int port, const int maxListenBacklog = MAX_LISTEN_BACKLOG);
			
			TDescriptor acceptDescriptor(TIPv4 &ip);
			bool setDeferAccept(const int timeOut);
			void reset(const TDescriptor descr);
			bool reopen();
			
			static const size_t DEFAULT_CONNECT_TIMEOUT = 15 * 1000;
			enum EConnectResult
			{
				CN_ERORR,
				CN_CONNECTED,
				CN_NOT_READY,
				CN_NEED_RESET,
			};
			EConnectResult connectNonBlock(const TIPv4 ip, const uint32_t port);
			bool connect(const TIPv4 ip, const uint32_t port, const size_t timeout = DEFAULT_CONNECT_TIMEOUT);
			
			const TDescriptor descr() const
			{
				return _descr;
			}
			static const size_t DEFAULT_READ_TIMEOUT = 60 * 1000; // 60 seconds
			bool pollAndRecvAll(void *buf, const size_t size, const size_t timeout = DEFAULT_READ_TIMEOUT);
			ssize_t pollAndrecv(void *buf, const size_t size, const size_t timeout = DEFAULT_READ_TIMEOUT);
			static const size_t DEFAULT_SEND_TIMEOUT = 30 * 1000; // 30 seconds
			bool pollAndSendAll(const void *buf, const size_t size, const size_t timeout = DEFAULT_SEND_TIMEOUT);
			bool pollReadHttpAnswer(BString &answer, const size_t timeout = DEFAULT_READ_TIMEOUT);
			int getFlags();
			static fl::strings::BString ip2String(const TIPv4 ip);
			static TIPv4 ip2Long(const char *ipStr);
			static bool setNonBlockIO(const TDescriptor descr);
			static bool setNoDelay(const TDescriptor descr, int flag);
			bool setNonBlockIO()
			{
				return setNonBlockIO(_descr);
			}
			static TIPv4 resolve(const char *host, BString &buf);
		private:
			bool _open();
			TDescriptor _descr;
		};
	}
};

#endif	// __FL_SOCKET_HPP
