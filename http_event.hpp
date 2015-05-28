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
			virtual ~HttpEventInterface() 
			{
			}
			virtual bool parseURI(const char *cmdStart, const EHttpVersion::EHttpVersion version,
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
			virtual bool formError(class BString &result, class HttpEvent *http)
			{
				return false;
			}
			virtual bool canContinue()
			{
				return true;
			}
			enum EFormResult
			{
				 RESULT_OK_CLOSE = 0,
				 RESULT_OK_KEEP_ALIVE,
				 RESULT_OK_WAIT,
				 RESULT_ERROR,
				 RESULT_FINISH,
				 RESULT_OK_PARTIAL_SEND,
				 RESULT_SKIP,
			};
			virtual EFormResult formResult(BString &networkBuffer, class HttpEvent *http) = 0;
			virtual bool reset()
			{
				return false;
			}
			virtual EFormResult getMoreDataToSend(BString &networkBuffer, class HttpEvent *http)
			{
				return RESULT_FINISH;
			}
		protected:
			static bool _parseKeepAlive(const char *name, const size_t nameLength, const char *value, bool &isKeepAlive);
			static bool _parseIfModifiedSince(const char *name, const size_t nameLength, 
				const char *value, const size_t valueLen, time_t &ifModifiedSince);
			static bool _parseContentLength(const char *name, const size_t nameLength, const char *value, 
				size_t &contentLength);
			static bool _parseHost(const char *name, const size_t nameLength, const char *value, const size_t valueLen, 
				std::string &host);
			static bool _parseRange(const char *name, const size_t nameLength, const char *value, const size_t valueLen,
				int32_t &rangeStart, uint32_t &rangeEnd);
			static bool _isCookieHeader(const char *name, const size_t nameLength);
			static const char _nextParam(const char *&paramStart, const char *end, const char *&value, size_t &valueLength);
			enum class EHttpRequestType : uint8_t
			{
				UNKNOWN,
				GET,
				POST,
				HEAD,
			};
			static EHttpRequestType _parseHTTPCmd(const char cmdStart); 
			static void _addConnectionHeader(BString &networkBuffer, const bool isKeepAlive);
			
			enum EError : uint8_t
			{
				ERROR_200_OK = 0,
				ERROR_202_ACCEPTED,
				ERROR_204_NO_CONTENT,
				ERROR_206_PARTIAL_CONTENT,
				ERROR_400_BAD_REQUEST,
				ERROR_401_UNAUTHORIZED,
				ERROR_404_NOT_FOUND,
				ERROR_405_METHOD_NOT_ALLOWED,
				ERROR_409_CONFLICT,
				ERROR_410_GONE,
				ERROR_411_LENGTH_REQUIRED,
				ERROR_498_TOKEN_EXPIRED,
				ERROR_500_INTERNAL_SERVER_ERROR,
				ERROR_503_SERVICE_UNAVAILABLE,
				ERROR_505_HTTP_VERSION_NOT_SUPPORTED,
				ERROR_507_INSUFFICIENT_STORAGE,
				ERROR_MAX,
			};
			static const std::string _ERROR_STRINGS[ERROR_MAX];

		};
		
		class HttpEvent : public WorkEvent
		{
		public:
			HttpEvent(const TEventDescriptor descr, const time_t timeOutTime, HttpEventInterface *interface);
			virtual ~HttpEvent();
			virtual const ECallResult call(const TEvents events);
			NetworkBuffer *networkBuffer()
			{
				return _networkBuffer;
			}
			HttpEvent::ECallResult sendAnswer(const HttpEventInterface::EFormResult result);
			typedef uint8_t TStatus;
			static const TStatus ST_KEEP_ALIVE = 0x1;
			static const TStatus ST_CHECK_AFTER_SEND = 0x2;
			static const TStatus ST_EXPECT_100 = 0x4;
			
			void setKeepAlive()
			{
				_status |= ST_KEEP_ALIVE;
			}
			void clearKeepAlive()
			{
				_status &= (~ST_KEEP_ALIVE);
			}
			bool unAttach();
			bool attachAndSendAnswer(const HttpEventInterface::EFormResult result);
		private:
			bool _readRequest();
			bool _parseURI(const char *beginURI, const char *endURI);
			bool _parseHeader(const char *pStartHeader, const char *pEndHeader);
			void _endWork();
			bool _readPostData();
			ECallResult _send100Continue();
			ECallResult _sendAnswer();
			ECallResult _sendError();
			void _updateTimeout();
			bool _reset();
			const ECallResult _setWaitExternalEvent();
			bool _checkExpect(const char *name, const size_t nameLength, const char *value, const size_t valueLen);
			
			HttpEventInterface *_interface;
			NetworkBuffer *_networkBuffer;
			uint32_t _headerStartPosition;
			enum EHttpState : uint8_t
			{
				ST_WAIT_REQUEST,
				ST_WAIT_ADDITIONAL_DATA,
				ST_REQUEST_RECEIVED,
				ST_SEND,
				ST_WAIT_EXTERNAL_EVENT,
				ST_UNATTACHED,
				ST_FINISHED
			};
			EHttpState _state;
			uint8_t _chunkNumber;
			TStatus _status;
		};

		class HttpThreadSpecificData : public ThreadSpecificData
		{
		public:
			HttpThreadSpecificData(const NetworkBuffer::TSize maxRequestSize = 1024 * 1024, const uint8_t maxChunkCount = 128, 
				const size_t bufferSize = 32 * 1024, const size_t maxFreeBuffers = 1024, 
				const uint32_t operationTimeout = 60, const uint32_t firstRequstTimeout = 15, const uint32_t keepAlive = 60);
			virtual ~HttpThreadSpecificData() {}
			NetworkBuffer::TSize maxRequestSize;
			uint8_t maxChunkCount;
			NetworkBufferPool bufferPool;
			uint32_t operationTimeout;
			uint32_t firstRequstTimeout;
			uint32_t keepAlive;
		};

	};
};

#endif	// __FL_HTTP_EVENT_HPP
