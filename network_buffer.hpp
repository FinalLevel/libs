#pragma once
#ifndef __FL_NETWORK_BUFFER_HPP
#define	__FL_NETWORK_BUFFER_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Class implements buffered network functions.
///////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <vector>
#include "bstring.hpp"
#include "socket.hpp"

namespace fl {
	namespace network {
		class NetworkBuffer : public fl::strings::BString
		{
		public:
			NetworkBuffer(const TSize reserved = DEFAULT_RESERVED_SIZE)
				: BString(reserved), _sended(0)
			{
			}

			enum EResult
			{
				OK,
				IN_PROGRESS,
				CONNECTION_CLOSE,
				ERROR
			};

			EResult send(const TDescriptor descr);
			EResult read(const TDescriptor descr);
			void clear()
			{
				_sended = 0;
				BString::clear();
			}
		protected:
			TSize _sended;
		};
		
		class NetworkBufferPool
		{
		public:
			NetworkBufferPool(const int bufferSize, const uint32_t freeBuffersLimit);
			NetworkBuffer *get();
			void free(NetworkBuffer *buf);
		private:
			int _bufferSize;
			uint32_t _freeBuffersLimit;
			
			typedef std::vector<NetworkBuffer*> TNetworkBufferVector;
			TNetworkBufferVector _freeBuffers;
		};
	};
};

#endif	// __FL_NETWORK_BUFFER_HPP
