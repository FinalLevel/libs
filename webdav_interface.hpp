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

namespace fl {
	namespace http {
		using namespace fl::events;
		
		class WebDavInterface : public HttpEventInterface
		{
		public:
			WebDavInterface();
			virtual ~WebDavInterface() 
			{
			}
			virtual bool parseURI(const char *cmdStart, const EHttpVersion::EHttpVersion version,
				const std::string &host, const std::string &fileName, const std::string &query);
			virtual bool parsePOSTData(const uint32_t postStartPosition, NetworkBuffer &buf, bool &parseError);
			virtual bool parseHeader(const char *name, const size_t nameLength, const char *value, const size_t valueLen, 
				const char *pEndHeader);
			virtual bool formError(EHttpState::EHttpState &state, BString &result);
			
			virtual EFormResult formResult(BString &networkBuffer, class HttpEvent *http) = 0;
			virtual bool reset();
		protected:
			typedef uint8_t TStatus;
			TStatus _status;
			static const TStatus ST_OVERWRITE = 0x1;
			static const TStatus ST_KEEP_ALIVE = 0x2;
			
			// the statuses from 0x10 to 0x80 are reserved for CMD specific purposes
			static const TStatus ST_PROP_FIND_SUPPORTED_METHOD_SET				= 0x10;
			
			static const std::string HTTP_MULTI_STATUS;

			enum EError : uint8_t
			{
				ERROR_NO = 0,
				ERROR_BAD_REQUEST,
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

			
			virtual bool _savePartialPOSTData(const uint32_t postStartPosition, NetworkBuffer &buf, bool &parseError);
			virtual bool _put(const char *dataStart);
			
			bool _parsePropFind(const char *data);
			
			virtual bool _parsePropFindProperty(const char *propertyName);
			
			std::string _host;
			std::string _fileName;
		};
	};
};

#endif	// __FL_WEBDAV_INTERFACE_HPP
