#pragma once
#ifndef __FL_HTTP_EVENT_HPP
#define	__FL_HTTP_EVENT_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Http events system implementation
///////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include "event_thread.hpp"
#include "network_buffer.hpp"
#include "bstring.hpp"

namespace fl {
	namespace events {
		using fl::network::NetworkBufferPool;
		using fl::strings::BString;
		
		namespace EHttpRequestType
		{
			enum EHttpRequestType 
			{
				GET,
				POST,
				HEAD,
			};
		};
		namespace EHttpState
		{
			enum EHttpState : uint8_t
			{
				ST_WAIT_REQUEST,
				ST_WAIT_ADDITIONAL_DATA,
				ST_REQUEST_RECEIVED,
				ST_SEND,
				ST_SEND_AND_CLOSE,
				ST_FINISHED
			};
		};
		namespace EHttpVersion
		{
			enum EHttpVersion : uint8_t
			{
				HTTP_1_0,
				HTTP_1_1,
			};
		};

		class HttpEventInterface
		{
		public:
			virtual bool parseURI(const EHttpRequestType::EHttpRequestType reqType, const EHttpVersion::EHttpVersion version,
				const std::string &host, const std::string &fileName, const std::string &query) = 0;
			virtual bool parsePOSTData(const uint32_t postStartPosition, NetworkBuffer &buf, bool &parseError)
			{
				return true;
			}
			virtual bool parseHeader(const char *name, const size_t nameLength, const char *value, const size_t valueLen, 
				const char *pEndHeader)
			{
				return true;
			}
			virtual bool formError(const EHttpState::EHttpState state, BString &result)
			{
				return false;
			}
			enum EFormResult
			{
				 RESULT_OK_CLOSE = 0,
				 RESULT_OK_KEEP_ALIVE,
				 RESULT_ERROR,
			};
			virtual EFormResult formResult(BString &networkBuffer, class HttpEvent *http) = 0;
		};
		
		class HttpEvent : public WorkEvent
		{
		public:
			HttpEvent(const TEventDescriptor descr, const time_t timeOutTime, HttpEventInterface *interface);
			virtual ~HttpEvent();
			virtual const ECallResult call(const TEvents events);
		private:
			static const char *_terminatingCharacters;
			bool _readRequest();
			bool _parseURI(const char *beginURI, const char *endURI);
			bool _parseHeader(const char *pStartHeader, const char *pEndHeader);
			void _endWork();
			bool _readPostData();
			ECallResult _sendAnswer();
			ECallResult _sendError();
			
			HttpEventInterface *_interface;
			NetworkBuffer *_networkBuffer;
			uint32_t _headerStartPosition;
			EHttpState::EHttpState _state;
			uint8_t _endCharacterNumber;
			uint8_t _chunkNumber;
			typedef uint8_t TStatus;
			TStatus _status;
		};

		class HttpThreadSpecificData : public ThreadSpecificData
		{
		public:
			HttpThreadSpecificData(const NetworkBuffer::TSize maxRequestSize = 1024 * 1024, const uint8_t maxChunkCount = 128, 
				const size_t bufferSize = 32 * 1024, const size_t maxFreeBuffers = 1024);
			virtual ~HttpThreadSpecificData() {}
			uint8_t maxChunkCount;
			NetworkBuffer::TSize maxRequestSize;
			NetworkBufferPool bufferPool;
		};

	};
};

#endif	// __FL_HTTP_EVENT_HPP
