#pragma once
#ifndef __FL_WEBDAV_INTERFACE_HPP
#define	__FL_WEBDAV_INTERFACE_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: WebDAV http extension classes
///////////////////////////////////////////////////////////////////////////////

#include "http_event.hpp"
#include "file.hpp"

namespace fl {
	namespace http {
		using namespace fl::events;
		using fl::fs::File;
		
		const size_t DEFALUT_POST_INMEMMORY_SIZE = 1024 * 64; // 64 kByte
		
		class WebDavInterface : public HttpEventInterface
		{
		public:
			WebDavInterface();
			virtual ~WebDavInterface() {};

			virtual bool parseURI(const char *cmdStart, const EHttpVersion::EHttpVersion version,
				const std::string &host, const std::string &fileName, const std::string &query);
			virtual bool parsePOSTData(const uint32_t postStartPosition, NetworkBuffer &buf, bool &parseError);
			virtual bool parseHeader(const char *name, const size_t nameLength, const char *value, const size_t valueLen, 
				const char *pEndHeader);
			virtual bool formError(EHttpState::EHttpState &state, BString &result);
			
			virtual EFormResult formResult(BString &networkBuffer, class HttpEvent *http);
			virtual bool reset();
			
			static void setMaxPostInMemmorySize(const size_t maxPostInMemmorySize)
			{
				_maxPostInMemmorySize = maxPostInMemmorySize;
			}
			static size_t maxPostInMemmorySize()
			{
				return _maxPostInMemmorySize;
			}
		protected:
			static size_t _maxPostInMemmorySize;
			static std::string _tmpPath;
			
			
			typedef uint8_t TStatus;
			TStatus _status;
			static const TStatus ST_OVERWRITE = 0x1;
			static const TStatus ST_KEEP_ALIVE = 0x2;
			
			// the statuses from 0x10 to 0x80 are reserved for CMD specific purposes
			static const TStatus ST_PROP_FIND_SUPPORTED_METHOD_SET				= 0x10;
			
			static const TStatus ST_POST_SPLITED													= 0x10;
			
			static const std::string HTTP_MULTI_STATUS;
			static const std::string HTTP_CREATED_STATUS;

			enum EError : uint8_t
			{
				ERROR_200_OK = 0,
				ERROR_400_BAD_REQUEST,
				ERROR_405_METHOD_NOT_ALLOWED,
				ERROR_409_CONFLICT,
				ERROR_411_LENGTH_REQUIRED,
				ERROR_503_SERVICE_UNAVAILABLE,
				ERROR_507_INSUFFICIENT_STORAGE,
				ERROR_MAX,
			};
			static const std::string _ERROR_STRINGS[ERROR_MAX];
			EError _error;
			enum class ERequestType : uint8_t
			{
				UNKNOWN,
				GET,
				DELETE,
				MKCOL,
				PUT,
				OPTIONS,
				PROPFIND,
				HEAD,
			};
			ERequestType _requestType;
			bool _parseRequestType(const char *cmdStart);
			
			size_t _contentLength;

			
			bool _parseOverwrite(const char *name, const size_t nameLength, const char *value, const char *pEndHeader);
			EFormResult _formOptions(BString &networkBuffer);
			
			static const std::string SUPPORTED_METHOD_SET;
			EFormResult _formPropFind(BString &networkBuffer);

			
			bool _savePartialPOSTData(const uint32_t postStartPosition, NetworkBuffer &buf, bool &parseError);
			BString _putData;
			virtual EFormResult _formPut(BString &networkBuffer, class HttpEvent *http);
			
			bool _parsePropFind(const char *data);
			
			virtual bool _parsePropFindProperty(const char *propertyName);
			
			std::string _host;
			std::string _fileName;
			
			EFormResult _keepAliveState()
			{
				return (_status & ST_KEEP_ALIVE) ? EFormResult::RESULT_OK_KEEP_ALIVE : EFormResult::RESULT_OK_CLOSE;
			}
			bool _savePostChunk(const char *data, const size_t size);
			File _postTmpFile;
			
			EFormResult _formMkCOL(BString &networkBuffer);
			virtual bool _mkCOL()
			{
				return false;
			}
		};
	};
};

#endif	// __FL_WEBDAV_INTERFACE_HPP
