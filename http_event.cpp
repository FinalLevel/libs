///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Http events system implementation
///////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include "http_event.hpp"
#include "log.hpp"
#include "time.hpp"
#include "util.hpp"

using namespace fl::events;


HttpEvent::HttpEvent(const TEventDescriptor descr, const time_t timeOutTime, HttpEventInterface *interface)
	: WorkEvent(descr, timeOutTime), _interface(interface), _networkBuffer(NULL), _headerStartPosition(0),
		_state(EHttpState::ST_WAIT_REQUEST), _chunkNumber(0), _status(0)
{
	setWaitRead();
}

bool HttpEvent::_reset()
{
	if (_interface->reset()) {
		_state = EHttpState::ST_WAIT_REQUEST;
		auto threadSpecData = static_cast<HttpThreadSpecificData*>(_thread->threadSpecificData());
		if (_networkBuffer) {
			threadSpecData->bufferPool.free(_networkBuffer);
			_networkBuffer = NULL;
		}
		_headerStartPosition = 0;
		_chunkNumber = 0;
		_status = 0;
		setWaitRead();
		if (!_thread->ctrl(this))
			return false;
		_timeOutTime = EPollWorkerGroup::curTime.unix() + threadSpecData->keepAlive;
		return true;
	} else
		return false;	
}

HttpEvent::~HttpEvent()
{
	_endWork();
	delete _interface;
}

void HttpEvent::_endWork()
{
	_state = EHttpState::ST_FINISHED;
	_timeOutTime = 0;
	if (_descr != 0) {
		close(_descr);
		_descr = 0;
	}
	if (_networkBuffer) {
		auto threadSpecData = static_cast<HttpThreadSpecificData*>(_thread->threadSpecificData());
		threadSpecData->bufferPool.free(_networkBuffer);
		_networkBuffer = NULL;
	}
}

bool HttpEvent::_parseURI(const char *beginURI, const char *endURI)
{
	auto cmdEnd = beginURI;
	while (cmdEnd < endURI) {
		if (*cmdEnd == ' ')
			break;
		cmdEnd++;
	}
	if (cmdEnd == endURI) {
		log::Error::L("Can't find an URI begin\n");
		return false;
	}
	auto cmdStart = beginURI;
	beginURI = cmdEnd + 1;
	
	static const std::string HTTP_VERSION("HTTP/");
	const char *endURL =  endURI - HTTP_VERSION.size() - 3; // - major.minor (1.1)
	EHttpVersion::EHttpVersion version = EHttpVersion::HTTP_1_0; 
	if (!strncasecmp(endURL, HTTP_VERSION.c_str(), HTTP_VERSION.size())) {
		if (*(endURI - 1) == '1')
			version = EHttpVersion::HTTP_1_1;
		endURL--;
		if (*endURL != ' ') {
			log::Error::L("Space expected in HTTP request\n");
			return false;
		}
	} else {
		log::Error::L("Can't find 'HTTP/' in HTTP request\n");
		return false;
	}
	while (endURL > beginURI)	{
		if (*endURL == ' ')
			break;
		endURL--;
	}
	if (endURL == beginURI) {
		log::Error::L("Size of URL cannot be zero\n");
		return false;
	}
	
	static const std::string PROTOCOL_HTTP("http://");
	static const std::string PROTOCOL_HTTPS("https://");
	uint32_t skipedCharacters = 0;
	if (!strncasecmp(beginURI, PROTOCOL_HTTP.c_str(), PROTOCOL_HTTP.size()))
		skipedCharacters = PROTOCOL_HTTP.size();
	else if (!strncasecmp(beginURI, PROTOCOL_HTTPS.c_str(), PROTOCOL_HTTPS.size()))
		skipedCharacters = PROTOCOL_HTTPS.size();
	
	std::string hostName;
	if (skipedCharacters > 0)	{
		beginURI += skipedCharacters;
		const char *pBeginHost = beginURI;
		while (beginURI < endURL) {
			if ((*beginURI == '/') || (*beginURI == ':')) {
				int len = beginURI - pBeginHost;
				if (len > 0)
					hostName.assign(pBeginHost, len);
				if (*beginURI == ':') {
					while (beginURI < endURL) {
						if (*beginURI == '/')
							break;
						beginURI++;
					}
				}
				break;
			}
			beginURI++;
		}
	}
	const char *pQuery = beginURI;
	while (pQuery < endURL) {
		if (*pQuery == '?')
			break;
		pQuery++;
	}

	std::string query;
	int queryLen = endURL - pQuery;
	if (queryLen > 1)
		query.assign(pQuery + 1, queryLen - 1); // skip '?'

	std::string fileName;
	int fileNameLen = pQuery - beginURI;
	if (fileNameLen > 0)
		fileName.assign(beginURI, fileNameLen);
	return _interface->parseURI(cmdStart, version, hostName, fileName, query);	
}

bool HttpEvent::_checkExpect(const char *name, const size_t nameLength, const char *value, const size_t valueLen)
{
	static const std::string EXPECT_HEADER("Expect");
	if (nameLength != EXPECT_HEADER.size())
		return false;
	if (strncasecmp(name, EXPECT_HEADER.c_str(), EXPECT_HEADER.size()))
		return false;
	else {
		static const std::string VALUE_100_CONTINUE("100-Continue");
		if (!strncasecmp(value, VALUE_100_CONTINUE.c_str(), VALUE_100_CONTINUE.size())) {
			_status |= ST_EXPECT_100; 
			return true;
		} else {
			return false;
		}
	}
}

bool HttpEvent::_parseHeader(const char *pStartHeader, const char *pEndHeader)
{
	const char *pBeginName = pStartHeader;
	while (pStartHeader < pEndHeader)
	{
		if (*pStartHeader == ':')
		{
			int nameLength = pStartHeader - pBeginName;
			if (nameLength <= 0)
				return false;
			pStartHeader++;
			while (isspace(*pStartHeader) && (pStartHeader < pEndHeader))
				pStartHeader++;
			int valueLen = pEndHeader - pStartHeader;
			if (valueLen <= 0)
				return true;
			if (_checkExpect(pBeginName, nameLength, pStartHeader, valueLen)) {
				return _interface->canContinue();
			}
			else {	
				return _interface->parseHeader(pBeginName, nameLength, pStartHeader, valueLen, pEndHeader);
			}
		}
		pStartHeader++;
	}
	return false;
}

bool HttpEvent::_readRequest()
{
	auto threadSpecData = static_cast<HttpThreadSpecificData*>(_thread->threadSpecificData());
	if (!_networkBuffer) {
		_networkBuffer = threadSpecData->bufferPool.get();
	}

	auto lastChecked = _networkBuffer->size();
	auto res = _networkBuffer->read(_descr);
	if ((res == NetworkBuffer::ERROR) || (res == NetworkBuffer::CONNECTION_CLOSE))
		return false;
	else if (res == NetworkBuffer::IN_PROGRESS)
		return true;

	_chunkNumber++;
	if (_chunkNumber > threadSpecData->maxChunkCount) {
		log::Error::L("Too many chunks (%u) received during a request reading\n", _chunkNumber);
		return false;
	}
	if ((_state == EHttpState::ST_WAIT_REQUEST) && (_networkBuffer->size() > threadSpecData->maxRequestSize)) {
		log::Error::L("Maximum http request size %u has been reached (%u)\n", threadSpecData->maxRequestSize, 
			_networkBuffer->size());
		return false;
	}
	static const NetworkBuffer::TSize MIN_HTTP_REQUEST = sizeof("GET / HTTP/1.0\r\n\r\n") - 2;
	if (_networkBuffer->size() > MIN_HTTP_REQUEST)	{
		if (lastChecked > 0) // skip one char because it might be '\r' before '\n'
			lastChecked--;
		const char *pBuffer = _networkBuffer->c_str() + lastChecked;
		bool fullRequestFound = false;
		while (*pBuffer != 0) {
			if (*pBuffer == '\r')
			{
				pBuffer++;
				if (*pBuffer == '\n') {
					pBuffer++;
					const char *pEndHeader = pBuffer - 2; // skip \r\n
					const char *pBeginHeader = _networkBuffer->c_str() + _headerStartPosition;
					int headerLength = pEndHeader - pBeginHeader;
					if (!_headerStartPosition) { // parse URI
						if (!_parseURI(pBeginHeader, pEndHeader))
							return false;
					} else if (headerLength > 0) {
						if (!_parseHeader(pBeginHeader, pEndHeader))
							return false;
					} else  {// \r\n\r\n found
						_headerStartPosition = pBuffer - _networkBuffer->c_str();
						fullRequestFound = true;
						break;
					}
					_headerStartPosition = pBuffer - _networkBuffer->c_str();
				}
			}
			else
				pBuffer++;
		}
		if (fullRequestFound) { // end query was found
			bool parseError = false;
			if (_interface->parsePOSTData(_headerStartPosition, *_networkBuffer, parseError)) {
				_state = EHttpState::ST_REQUEST_RECEIVED;
			}
			else {// Not enough data for post query or an error has been occurred
				if (parseError)
					return false;
				else
					_state = EHttpState::ST_WAIT_ADDITIONAL_DATA;
			}			
		
		}
	}
	return true;
}

void HttpEvent::_updateTimeout()
{
	auto threadSpecData = static_cast<HttpThreadSpecificData*>(_thread->threadSpecificData());
	_timeOutTime = EPollWorkerGroup::curTime.unix() + threadSpecData->operationTimeout;	
}

HttpEvent::ECallResult HttpEvent::_sendAnswer()
{
	auto res = _networkBuffer->send(_descr);
	if (res == NetworkBuffer::IN_PROGRESS) {
		setWaitSend();
		if (_thread->ctrl(this)) {
			_updateTimeout();
			return CHANGE;
		}
		else
			return FINISHED;
	} else if (res == NetworkBuffer::OK) {
		if (_status & ST_CHECK_AFTER_SEND) {
			_networkBuffer->clear();
			return sendAnswer(_interface->getMoreDataToSend(*_networkBuffer, this));
		}
		if (_status & ST_KEEP_ALIVE) {
			if (_reset())
				return CHANGE;
		}
		if (_status & ST_EXPECT_100) {
			setWaitRead();
			if (_thread->ctrl(this)) {
				_updateTimeout();
				_status &= (~ST_EXPECT_100);
				_state = EHttpState::ST_WAIT_ADDITIONAL_DATA;
				return CHANGE;
			}
		}
	}
	return FINISHED;	
}

HttpEvent::ECallResult HttpEvent::_sendError()
{
	_state = ST_SEND;
	_status &= ~(ST_KEEP_ALIVE);
	_networkBuffer->clear();
	if (!_interface->formError(*_networkBuffer, this))
		_networkBuffer->sprintfSet("HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n");
	return _sendAnswer();
}

bool HttpEvent::_readPostData()
{
	auto res = _networkBuffer->read(_descr);
	if ((res == NetworkBuffer::ERROR) || (res == NetworkBuffer::CONNECTION_CLOSE))
		return false;
	else if (res == NetworkBuffer::IN_PROGRESS)
		return true;
	
	bool parseError = false;
	if (_interface->parsePOSTData(_headerStartPosition, *_networkBuffer, parseError)) {
		_state = EHttpState::ST_REQUEST_RECEIVED;
	}
	else if (parseError) {// Not enough data for post query or an error has been occurred
		return false;
	}			
	return true;
}

bool HttpEvent::attachAndSendAnswer(const HttpEventInterface::EFormResult result)
{
	fl::threads::WeekAutoMutex autoSync;
	_updateTimeout();
	if (!_thread->addEvent(this, autoSync)) {
		// free buffer to prevent race condition on NetworkBufferPool 
		delete _networkBuffer;
		_networkBuffer = nullptr;
		return false;
	}
	sendAnswer(result);
	return true;
}

bool HttpEvent::unAttach()
{
	if (_thread->unAttachNL(this)) {
		_state = EHttpState::ST_UNATTACHED;
		return true;
	} else {
		return false;
	}
}
const HttpEvent::ECallResult HttpEvent::_setWaitExternalEvent()
{
	_state = EHttpState::ST_WAIT_EXTERNAL_EVENT;
	_events = E_ERROR | E_HUP;
	if (_thread->ctrl(this)) {
		_updateTimeout();
		return CHANGE;
	}
	else
		return FINISHED;	
}

HttpEvent::ECallResult HttpEvent::sendAnswer(const HttpEventInterface::EFormResult result)
{
	_status &= ~(ST_KEEP_ALIVE | ST_CHECK_AFTER_SEND);
	
	ECallResult sendResult = SKIP;
	switch (result) {
		case HttpEventInterface::RESULT_OK_KEEP_ALIVE:
			_state = EHttpState::ST_SEND;
			_status |= ST_KEEP_ALIVE;
			sendResult = _sendAnswer();
		break;
		case HttpEventInterface::RESULT_OK_CLOSE:
			_state = EHttpState::ST_SEND;
			sendResult = _sendAnswer();
		break;
		case HttpEventInterface::RESULT_OK_PARTIAL_SEND:
			_state = EHttpState::ST_SEND;
			_status |= ST_CHECK_AFTER_SEND;
			sendResult = _sendAnswer();
		break;
		case HttpEventInterface::RESULT_OK_WAIT:
			sendResult = _setWaitExternalEvent();
		break;
		case HttpEventInterface::RESULT_ERROR:
			sendResult = _sendError();
		break;
		case HttpEventInterface::RESULT_FINISH:
			sendResult = FINISHED;
		break;
		case HttpEventInterface::RESULT_SKIP:
			return SKIP;
	};
	if (sendResult == FINISHED)
		_endWork();
	return sendResult;
}

HttpEvent::ECallResult HttpEvent::_send100Continue()
{
	_state = EHttpState::ST_SEND;
	_networkBuffer->clear();
	static const std::string HTTP_CONTINUE_HEADER("HTTP/1.1 100 Continue\r\n\r\n");
	*_networkBuffer << HTTP_CONTINUE_HEADER;
	_headerStartPosition = _networkBuffer->size();
	return _sendAnswer();
}

const HttpEvent::ECallResult HttpEvent::call(const TEvents events)
{
	if (_state == EHttpState::ST_FINISHED)
		return FINISHED;
	
	if (((events & E_HUP) == E_HUP) || ((events & E_ERROR) == E_ERROR)) {
		_endWork();
		return FINISHED;
	}
	
	if (events & E_INPUT) {
		if (_state == EHttpState::ST_WAIT_REQUEST)	{
			if (!_readRequest())
				return _sendError();
		}
		if (_state == EHttpState::ST_WAIT_ADDITIONAL_DATA) {
			if (_status & ST_EXPECT_100) {
				return _send100Continue();
			} else if (!_readPostData()) {
				return _sendError();
			}
		}
		if (_state == EHttpState::ST_REQUEST_RECEIVED) {
			_networkBuffer->clear();
			return sendAnswer(_interface->formResult(*_networkBuffer, this));
		} else {
			_updateTimeout();
			return CHANGE;
		}
	}
	
	if (events & E_OUTPUT) {
		if (_state == EHttpState::ST_SEND) {
			return _sendAnswer();
		} else {
			log::Error::L("Output event is in error state (%u/%u)\n", _events, _state);
			return FINISHED;
		}
	}
	return SKIP;
}

HttpThreadSpecificData::HttpThreadSpecificData(const NetworkBuffer::TSize maxRequestSize, const uint8_t maxChunkCount, 
	const size_t bufferSize, const size_t maxFreeBuffers, 
	const uint32_t operationTimeout, const uint32_t firstRequstTimeout, const uint32_t keepAlive)
	: maxRequestSize(maxRequestSize), maxChunkCount(maxChunkCount), bufferPool(bufferSize, maxFreeBuffers),
	operationTimeout(operationTimeout), firstRequstTimeout(firstRequstTimeout), keepAlive(keepAlive)
{
}


bool HttpEventInterface::_parseIfModifiedSince(const char *name, const size_t nameLength, 
	const char *value, const size_t valueLen, time_t &ifModifiedSince)
{
	static const std::string IF_MODIFIED_SINCE_HEADER("If-Modified-Since");
	if (nameLength != IF_MODIFIED_SINCE_HEADER.size())
		return false;
	if (strncasecmp(name, IF_MODIFIED_SINCE_HEADER.c_str(), IF_MODIFIED_SINCE_HEADER.size()))
		return false;
	else {
		ifModifiedSince = fl::chrono::Time::parseHttpDate(value, valueLen);
		return true;
	}
}

bool HttpEventInterface::_parseRange(const char *name, const size_t nameLength, const char *value, const size_t valueLen,
	int32_t &rangeStart, uint32_t &rangeEnd)
{
	static const std::string RANGE_HEADER("Range");
	if (nameLength != RANGE_HEADER.size())
		return false;
	if (strncasecmp(name, RANGE_HEADER.c_str(), RANGE_HEADER.size()))
		return false;
	else {
		rangeStart = 0;
		rangeEnd = 0;
		
		static const std::string BYTES("bytes=");
		const char *pBytes = fl::utils::strncasestr(value, BYTES.c_str(), valueLen);
		if (pBytes) {
			pBytes += BYTES.size();
			char *pEnd;
			rangeStart = strtol(pBytes, &pEnd, 10);
			if (*pEnd == '-') {
				rangeEnd = strtoul(pEnd + 1, NULL, 10);
				if (rangeEnd && (rangeStart > reinterpret_cast<decltype(rangeStart)>(rangeEnd))) {
					log::Warning::L("Received bad range: %u-%u\n", rangeStart, rangeEnd);
					rangeStart = 0;
					rangeEnd = 0;
				}
			}
		}
		return true;
	}	
}

bool HttpEventInterface::_parseKeepAlive(const char *name, const size_t nameLength, const char *value, 
	bool &isKeepAlive)
{
	static const std::string CONNECTION_HEADER("Connection");
	if (nameLength != CONNECTION_HEADER.size())
		return false;
	if (strncasecmp(name, CONNECTION_HEADER.c_str(), CONNECTION_HEADER.size()))
		return false;
	else {
		static const std::string KEEP_ALIVE_VALUE("keep-alive");
		if (strncasecmp(value, KEEP_ALIVE_VALUE.c_str(), KEEP_ALIVE_VALUE.size())) {
			isKeepAlive = false;
		} else {
			isKeepAlive = true;
		}
		return true;
	}
	
}


bool HttpEventInterface::_parseHost(const char *name, const size_t nameLength, const char *value, const size_t valueLen,
	std::string &host)
{
	static const std::string HOST_HEADER("Host");
	if (nameLength != HOST_HEADER.size())
		return false;
	if (strncasecmp(name, HOST_HEADER.c_str(), HOST_HEADER.size()))
		return false;
	else {
		host.assign(value, valueLen);
		return true;
	}

}

bool HttpEventInterface::_parseContentLength(const char *name, const size_t nameLength, const char *value, 
	size_t &contentLength)
{
	static const std::string CONTENT_LENGTH_HEADER("Content-length");
	if (nameLength != CONTENT_LENGTH_HEADER.size())
		return false;
	if (strncasecmp(name, CONTENT_LENGTH_HEADER.c_str(), CONTENT_LENGTH_HEADER.size()))
		return false;
	else {
		contentLength = strtoull(value, NULL, 10);
		return true;
	}
}

bool HttpEventInterface::_isCookieHeader(const char *name, const size_t nameLength)
{
	static const std::string COOKIE_HEADER("Cookie");
	if (nameLength != COOKIE_HEADER.size())
		return false;
	if (strncasecmp(name, COOKIE_HEADER.c_str(), COOKIE_HEADER.size()))
		return false;
	else
		return true;
}

const char HttpEventInterface::_nextParam(const char *&paramStart, const char *end, const char *&value, 
	size_t &valueLength)
{
	if (paramStart >= end)
		return 0;
	
	const char *pNext = strchr(paramStart, '&');
	if (!pNext)
		pNext = end;
	auto param = *paramStart;
	paramStart++;
	value = paramStart;
	valueLength = pNext - value;
	paramStart = pNext + 1;
	return param;
}

HttpEventInterface::EHttpRequestType HttpEventInterface::_parseHTTPCmd(const char cmdStart)
{
	auto firstChar = toupper(cmdStart);
	if (firstChar == 'G')	{
		return EHttpRequestType::GET;
	} else if (firstChar == 'P')	{
		return EHttpRequestType::POST;
	} else if (firstChar == 'H')	{
		return EHttpRequestType::HEAD;
	} else {
		log::Error::L("Not GET/POST/HEADER request has been received %c\n", firstChar);
		return EHttpRequestType::UNKNOWN;
	}
}

void HttpEventInterface::_addConnectionHeader(BString &networkBuffer, const bool isKeepAlive)
{
	if (isKeepAlive)
		networkBuffer << "Connection: Keep-Alive\r\n";
	else
		networkBuffer << "Connection: Close\r\n";
}

const std::string HttpEventInterface::_ERROR_STRINGS[ERROR_MAX] = {
	"HTTP/1.1 200 OK\r\n",
	"HTTP/1.1 204 No Content\r\n",
	"HTTP/1.1 206 Partial Content\r\n",
	"HTTP/1.1 400 Bad Request\r\n",
	"HTTP/1.1 401 Unauthorized\r\n",
	"HTTP/1.1 404 Not found\r\n",
	"HTTP/1.1 405 Method Not Allowed\r\n",
	"HTTP/1.1 409 Conflict\r\n",
	"HTTP/1.1 410 Gone\r\n",
	"HTTP/1.1 411 Length Required\r\n",
	"HTTP/1.1 500 Internal Server Error\r\n",
	"HTTP/1.1 503 Service Unavailable\r\n",
	"HTTP/1.1 505 HTTP Version Not Supported\r\n",
	"HTTP/1.1 507 Insufficient Storage\r\n",
};
