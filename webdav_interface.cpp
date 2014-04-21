///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: WebDAV http extension classes implementation
///////////////////////////////////////////////////////////////////////////////

#include "webdav_interface.hpp"
#include "log.hpp"
#include "rapidxml/rapidxml.hpp"
#include "http_answer.hpp"

using namespace rapidxml;
using namespace fl::http;

WebDavInterface::WebDavInterface()
	: _status(0), _error(EError::ERROR_NO), _requestType(ERequestType::UNKNOWN), _contentLength(0)
{
}

bool WebDavInterface::reset()
{
	if (_status & ST_KEEP_ALIVE) {
		_status = 0;
		_error = EError::ERROR_NO;
		_requestType = ERequestType::UNKNOWN;
		_contentLength = 0;
		_host.clear();
		_fileName.clear();
		return true;
	}
	else
		return false;
}

bool WebDavInterface::parseURI(const char *cmdStart, const EHttpVersion::EHttpVersion version, const std::string &host, 
	const std::string &fileName, const std::string &query)
{
	if (version != EHttpVersion::HTTP_1_1) {
		log::Error::L("WebDAV can work only over HTTP/1.1 protocol\n");
		_error = ERROR_BAD_REQUEST;
		return false;
	}
	_status |= ST_KEEP_ALIVE;
	if (!_parseRequestType(cmdStart))
	{
		_error = ERROR_BAD_REQUEST;
		return false;
	}
	_host = host;
	_fileName = fileName;
	return true;
}

bool WebDavInterface::parseHeader(const char *name, const size_t nameLength, const char *value, const size_t valueLen, 
				const char *pEndHeader)
{
	if (_parseContentLength(name, nameLength, value, _contentLength)) {
	} else if (_parseOverwrite(name, nameLength, value, pEndHeader)) {		
	} else {
		bool isKeepAlive = false;
		if (_parseKeepAlive(name, nameLength, value, isKeepAlive)) {
			if (isKeepAlive)
				_status |= ST_KEEP_ALIVE;
			else
				_status &= (~ST_KEEP_ALIVE);
		}
	}
	return true;
}

const std::string WebDavInterface::_ERROR_STRINGS[ERROR_MAX] = {
	"HTTP/1.1 200 OK\r\n",
	"HTTP/1.1 400 Bad Request\r\n",
};

bool WebDavInterface::formError(EHttpState::EHttpState &state, BString &result)
{
	result << _ERROR_STRINGS[_error];
	_addConnectionHeader(result, _status & ST_KEEP_ALIVE);
	result << "\r\n";
	state = (_status & ST_KEEP_ALIVE) ? EHttpState::ST_SEND : EHttpState::ST_SEND_AND_CLOSE;
	return true;
}

bool WebDavInterface::_savePartialPOSTData(const uint32_t postStartPosition, NetworkBuffer &buf, bool &parseError)
{
	if (postStartPosition + _contentLength <= (size_t)buf.size()) {	
		parseError = _put(buf.c_str() + postStartPosition);
		return true;
	}
	else
		return false;	
}

bool WebDavInterface::_parsePropFindProperty(const char *propertyName)
{
	return true;
}

bool WebDavInterface::_parsePropFind(const char *data)
{
	try
	{
		xml_document<> doc;
		doc.parse<parse_default>(const_cast<char*>(data));
		auto root = doc.first_node();
		if (root == NULL) {
			log::Error::L("WebDAV:PROPFIND: the root element is NULL\n");
			_error = ERROR_BAD_REQUEST;
			return false;
		}
		if (strcmp(root->name(), "D:propfind")) {
			log::Error::L("WebDAV:PROPFIND: bad root name %s\n", root->name());
			_error = ERROR_BAD_REQUEST;
			return false;			
		}
		for (auto child = root->first_node(); child != NULL; child = child->next_sibling()) {
			if (child->type() != node_element)
				continue;
			if (!strcmp(child->name(), "D:prop")) {
				for (auto prop = child->first_node(); prop != NULL; prop = prop->next_sibling()) {
					if (prop->type() != node_element)
						continue;	
					if (!strcmp(prop->name(), "supported-method-set")) {
						_status |= ST_PROP_FIND_SUPPORTED_METHOD_SET;
					} else if (!_parsePropFindProperty(prop->name()))
						return false;
				}
			}
		}
		return true;
	}
	catch (...)
	{
		log::Error::L("WebDAV:PROPFIND cannot parse XML\n");
		_error = ERROR_BAD_REQUEST;
		return false;
	}
}

bool WebDavInterface::parsePOSTData(const uint32_t postStartPosition, NetworkBuffer &buf, bool &parseError)
{
	parseError = false;
	if (_requestType == ERequestType::PUT) { // partial file storage may be required
		return _savePartialPOSTData(postStartPosition, buf, parseError);
	} else if (postStartPosition + _contentLength <= (size_t)buf.size()) {
		if (_requestType == ERequestType::PROPFIND)
			parseError = ! _parsePropFind(buf.c_str() + postStartPosition);
		return true;
	}
	else
		return false;	
}

HttpEventInterface::EFormResult WebDavInterface::formResult(BString &networkBuffer, class HttpEvent *http)
{
	switch (_requestType)
	{
	case ERequestType::GET:
		break;
	case ERequestType::PUT:
		break;
	case ERequestType::OPTIONS:
		return _formOptions(networkBuffer);
	case ERequestType::PROPFIND:
		return _formPropFind(networkBuffer);		
	case ERequestType::UNKNOWN:
		break;
	}
	return EFormResult::RESULT_ERROR;
}

const std::string WebDavInterface::HTTP_MULTI_STATUS = "HTTP/1.1 207 Multi-Status";


const std::string WebDavInterface::SUPPORTED_METHOD_SET = "<supported-method-set> \
  <supported-method name=\"COPY\" /> \
	<supported-method name=\"DELETE\" /> \
	<supported-method name=\"GET\" /> \
	<supported-method name=\"HEAD\" /> \
	<supported-method name=\"MKCOL\" /> \
	<supported-method name=\"MOVE\" /> \
	<supported-method name=\"OPTIONS\" /> \
	<supported-method name=\"POST\" /> \
	<supported-method name=\"PROPFIND\" /> \
 	<supported-method name=\"PROPPATCH\" /> \
	<supported-method name=\"PUT\" /> \
</supported-method-set>";

HttpEventInterface::EFormResult WebDavInterface::_formPropFind(BString &networkBuffer)
{
	HttpAnswer answer(networkBuffer, HTTP_MULTI_STATUS, "text/xml; charset=\"utf-8\"");
	networkBuffer << "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n<multistatus xmlns=\"DAV:\">";
	if (_status & (ST_PROP_FIND_SUPPORTED_METHOD_SET)) {
		networkBuffer << "<response><href>http://" << _host << _fileName << "</href><propstat><prop>";
		networkBuffer << SUPPORTED_METHOD_SET;
		networkBuffer << "</prop><status>HTTP/1.1 200 OK</status></propstat></response>";
	}
	networkBuffer << "</multistatus>";
	return (_status & ST_KEEP_ALIVE) ? EFormResult::RESULT_OK_KEEP_ALIVE : EFormResult::RESULT_OK_CLOSE;
}

HttpEventInterface::EFormResult WebDavInterface::_formOptions(BString &networkBuffer)
{
	networkBuffer << "HTTP/1.1 200 OK\r\n\
Allow: OPTIONS, GET, HEAD, POST, PUT, DELETE\r\n\
Allow: MKCOL, PROPFIND, PROPPATCH\r\n\
DAV: 1\r\n";
	_addConnectionHeader(networkBuffer, (_status & ST_KEEP_ALIVE));
	networkBuffer << "\r\n";
	return (_status & ST_KEEP_ALIVE) ? EFormResult::RESULT_OK_KEEP_ALIVE : EFormResult::RESULT_OK_CLOSE;
}

bool WebDavInterface::_parseOverwrite(const char *name, const size_t nameLength, const char *value, 
	const char *pEndHeader)
{
	static const std::string OVERWRITE_HEADER("Overwrite");
	if (nameLength != OVERWRITE_HEADER.size())
		return false;
	if (strncasecmp(name, OVERWRITE_HEADER.c_str(), OVERWRITE_HEADER.size()))
		return false;
	else {
		while (value < pEndHeader) {
			auto ch = toupper(*value);
			if (ch == 'F') {
				_status &= (~ST_OVERWRITE);
				break;
			} else if (ch == 'T') {
				_status |= ST_OVERWRITE;
				break;
			}
			value++;
		}
		return true;
	}
}

bool WebDavInterface::_parseRequestType(const char *cmdStart)
{
	static const std::string CMD_GET("GET");
	static const std::string CMD_DELETE("DELETE");
	static const std::string CMD_MKCOL("MKCOL");
	static const std::string CMD_PUT("PUT");
	static const std::string CMD_POST("POST");
	static const std::string CMD_OPTIONS("OPTIONS");
	static const std::string CMD_PROPFIND("PROPFIND");
	static const std::string CMD_HEAD("HEAD");
	if (strncasecmp(cmdStart, CMD_GET.c_str(), CMD_GET.size())) {
		_requestType = ERequestType::GET;
		return true;
	} else if (strncasecmp(cmdStart, CMD_HEAD.c_str(), CMD_HEAD.size())) {
		_requestType = ERequestType::HEAD;
		return true;				
	} else if (strncasecmp(cmdStart, CMD_DELETE.c_str(), CMD_DELETE.size())) {
		_requestType = ERequestType::DELETE;
		return true;
	} else if (strncasecmp(cmdStart, CMD_MKCOL.c_str(), CMD_MKCOL.size())) {
		_requestType = ERequestType::MKCOL;
		return true;
	} else if (strncasecmp(cmdStart, CMD_PUT.c_str(), CMD_PUT.size())) {
		_requestType = ERequestType::PUT;
		return true;
	} else if (strncasecmp(cmdStart, CMD_POST.c_str(), CMD_POST.size())) {
		_requestType = ERequestType::PUT;
		return true;
	} else if (strncasecmp(cmdStart, CMD_OPTIONS.c_str(), CMD_OPTIONS.size())) {
		_requestType = ERequestType::OPTIONS;
		return true;
	} else if (strncasecmp(cmdStart, CMD_PROPFIND.c_str(), CMD_PROPFIND.size())) {
		_requestType = ERequestType::PROPFIND;
		return true;				
	} else {
		log::Error::L("An unknown WebDAV request has been received %s\n", cmdStart);
		return false;	
	}
}
