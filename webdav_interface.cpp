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

size_t WebDavInterface::_maxPostInMemmorySize = DEFALUT_POST_INMEMMORY_SIZE;
std::string WebDavInterface::_tmpPath("/tmp");

WebDavInterface::WebDavInterface()
	: _status(0), _error(ERROR_400_BAD_REQUEST), _requestType(ERequestType::UNKNOWN), _contentLength(0)
{
}

bool WebDavInterface::reset()
{
	if (_status & ST_KEEP_ALIVE) {
		_status = 0;
		_error = ERROR_400_BAD_REQUEST;;
		_requestType = ERequestType::UNKNOWN;
		_contentLength = 0;
		_host.clear();
		_fileName.clear();
		_postTmpFile.close();
		_putData.clear();
		return true;
	}
	else
		return false;
}

bool WebDavInterface::reset(const HttpEventInterface::EFormResult result)
{
	return reset();
}

bool WebDavInterface::parseURI(const char *cmdStart, const EHttpVersion::EHttpVersion version, const std::string &host, 
	const std::string &fileName, const std::string &query)
{
	if (version != EHttpVersion::HTTP_1_1) {
		log::Error::L("WebDAV can work only over HTTP/1.1 protocol\n");
		_error = ERROR_505_HTTP_VERSION_NOT_SUPPORTED;
		return false;
	}
	_status |= ST_KEEP_ALIVE;
	if (!_parseRequestType(cmdStart))
	{
		_error = ERROR_400_BAD_REQUEST;
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
	} else if (_parseHost(name, nameLength, value, valueLen, _host)) {		
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

bool WebDavInterface::formError(class BString &result, class HttpEvent *http)
{
	HttpAnswer answer(result, _ERROR_STRINGS[_error], "text/xml; charset=\"utf-8\"", (_status & ST_KEEP_ALIVE));
	answer.setContentLength();
	if (_status & ST_KEEP_ALIVE) {
		http->setKeepAlive();
	}
	return true;
}

bool WebDavInterface::_savePostChunk(const char *data, const size_t size)
{
	if (_postTmpFile.descr() == 0) {
		if (!_postTmpFile.createUnlinkedTmpFile(_tmpPath.c_str())) {
			log::Error::L("Can't create a temporary file at %s to save a POST request\n", _tmpPath.c_str());
			_error = ERROR_507_INSUFFICIENT_STORAGE;
			return false;
		}
	}
	if (_postTmpFile.write(data, size) != (ssize_t)size) {
		log::Error::L("Can't save a POST request to the temporary file at %s\n", _tmpPath.c_str());
		_error = ERROR_507_INSUFFICIENT_STORAGE;
		return false;
	}
	if (size >= _contentLength)
		_contentLength = 0;
	else
		_contentLength -= size;
	return true;
}

bool WebDavInterface::_savePartialPOSTData(const uint32_t postStartPosition, NetworkBuffer &buf, bool &parseError)
{
	if (_status & ST_POST_SPLITED) {
		if (_contentLength <= (size_t)buf.size()) {
			if (_savePostChunk(buf.c_str(), buf.size())) {
				return true;
			}
			else {
				parseError = true;
				return false;
			}
		} else if (buf.size() >= (buf.reserved() / 2)) {
			parseError = ! _savePostChunk(buf.c_str(), buf.size());
			buf.clear();
		}
		return false;
	} else {
		if (postStartPosition + _contentLength <= (size_t)buf.size()) {	
			_putData.add(buf.c_str() + postStartPosition, _contentLength);
			return true;
		}
		else {
			if (_contentLength > _maxPostInMemmorySize) {
				if (buf.size() >= (buf.reserved() / 2)) {
					_status |= ST_POST_SPLITED;
					size_t loaded = buf.size() - postStartPosition;
					parseError = ! _savePostChunk(buf.c_str() + postStartPosition, loaded);
					buf.clear();
				}
			}
			return false;	
		}
	}
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
			_error = ERROR_400_BAD_REQUEST;
			return false;
		}
		if (strcmp(root->name(), "propfind")) {
			log::Error::L("WebDAV:PROPFIND: bad root name %s\n", root->name());
			_error = ERROR_400_BAD_REQUEST;
			return false;			
		}
		for (auto child = root->first_node(); child != NULL; child = child->next_sibling()) {
			if (child->type() != node_element)
				continue;
			if (!strcmp(child->name(), "prop")) {
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
		_error = ERROR_400_BAD_REQUEST;
		return false;
	}
}

bool WebDavInterface::parsePOSTData(const uint32_t postStartPosition, NetworkBuffer &buf, bool &parseError)
{
	parseError = false;
	if (_requestType == ERequestType::PUT) { // partial file storage may be required
		return _savePartialPOSTData(postStartPosition, buf, parseError);
	} else if (postStartPosition + _contentLength <= (size_t)buf.size()) {
		if (_requestType == ERequestType::PROPFIND) {
			 if (!_parsePropFind(buf.c_str() + postStartPosition)) {
				 parseError = true;
				 return false;
			 }
		}
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
		return _formGet(networkBuffer, http);
	case ERequestType::PUT:
		return _formPut(networkBuffer, http);
	case ERequestType::OPTIONS:
		return _formOptions(networkBuffer);
	case ERequestType::DELETE:
		return _formDelete(networkBuffer, http);
	case ERequestType::PROPFIND:
		return _formPropFind(networkBuffer);
	case ERequestType::MKCOL:
		return _formMkCOL(networkBuffer);	
	case ERequestType::UNKNOWN:
		break;
	}
	return EFormResult::RESULT_ERROR;
}

const std::string WebDavInterface::HTTP_MULTI_STATUS = "HTTP/1.1 207 Multi-Status\r\n";
const std::string WebDavInterface::HTTP_CREATED_STATUS = "HTTP/1.1 201 Created\r\n"; 


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
	HttpAnswer answer(networkBuffer, HTTP_MULTI_STATUS, "text/xml; charset=\"utf-8\"", (_status & ST_KEEP_ALIVE));
	networkBuffer << "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n<multistatus xmlns=\"DAV:\">";
	if (_status & (ST_PROP_FIND_SUPPORTED_METHOD_SET)) {
		networkBuffer << "<response><href>http://" << _host << _fileName << "</href><propstat><prop>";
		networkBuffer << SUPPORTED_METHOD_SET;
		networkBuffer << "</prop><status>HTTP/1.1 200 OK</status></propstat></response>";
	}
	networkBuffer << "</multistatus>";
	answer.setContentLength();
	return _keepAliveState();
}

HttpEventInterface::EFormResult WebDavInterface::_formOptions(BString &networkBuffer)
{
	HttpAnswer answer(networkBuffer, HTTP_OK_STATUS, "text/xml; charset=\"utf-8\"", (_status & ST_KEEP_ALIVE));
	answer.addHeaders("Allow: OPTIONS, GET, HEAD, POST, PUT, DELETE\r\n\
Allow: MKCOL, PROPFIND, PROPPATCH\r\n\
DAV: 1\r\n");
	answer.setContentLength();
	return _keepAliveState();
}

HttpEventInterface::EFormResult WebDavInterface::_formMkCOL(BString &networkBuffer)
{
	if (_mkCOL()) {
		HttpAnswer answer(networkBuffer, HTTP_CREATED_STATUS, "text/xml; charset=\"utf-8\"", (_status & ST_KEEP_ALIVE));
		answer.setContentLength();
		return _keepAliveState();	
	} else {
		return EFormResult::RESULT_ERROR;
	}
}

HttpEventInterface::EFormResult WebDavInterface::_formGet(BString &networkBuffer, class HttpEvent *http)
{
	_error = ERROR_405_METHOD_NOT_ALLOWED;
	return EFormResult::RESULT_ERROR;
}

HttpEventInterface::EFormResult WebDavInterface::_formPut(BString &networkBuffer, class HttpEvent *http)
{
	HttpAnswer answer(networkBuffer, HTTP_CREATED_STATUS, "text/xml; charset=\"utf-8\"", (_status & ST_KEEP_ALIVE));
	answer.setContentLength();
	return _keepAliveState();	
}

HttpEventInterface::EFormResult WebDavInterface::_formDelete(BString &networkBuffer, class HttpEvent *http)
{
	HttpAnswer answer(networkBuffer, _ERROR_STRINGS[ERROR_204_NO_CONTENT], "text/xml; charset=\"utf-8\"", 
		(_status & ST_KEEP_ALIVE));
	answer.setContentLength();
	return _keepAliveState();
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
	if (!strncasecmp(cmdStart, CMD_GET.c_str(), CMD_GET.size())) {
		_requestType = ERequestType::GET;
		return true;
	} else if (!strncasecmp(cmdStart, CMD_HEAD.c_str(), CMD_HEAD.size())) {
		_requestType = ERequestType::HEAD;
		return true;				
	} else if (!strncasecmp(cmdStart, CMD_DELETE.c_str(), CMD_DELETE.size())) {
		_requestType = ERequestType::DELETE;
		return true;
	} else if (!strncasecmp(cmdStart, CMD_MKCOL.c_str(), CMD_MKCOL.size())) {
		_requestType = ERequestType::MKCOL;
		return true;
	} else if (!strncasecmp(cmdStart, CMD_PUT.c_str(), CMD_PUT.size())) {
		_requestType = ERequestType::PUT;
		return true;
	} else if (!strncasecmp(cmdStart, CMD_POST.c_str(), CMD_POST.size())) {
		_requestType = ERequestType::PUT;
		return true;
	} else if (!strncasecmp(cmdStart, CMD_OPTIONS.c_str(), CMD_OPTIONS.size())) {
		_requestType = ERequestType::OPTIONS;
		return true;
	} else if (!strncasecmp(cmdStart, CMD_PROPFIND.c_str(), CMD_PROPFIND.size())) {
		_requestType = ERequestType::PROPFIND;
		return true;				
	} else {
		log::Error::L("An unknown WebDAV request has been received %s\n", cmdStart);
		return false;	
	}
}
