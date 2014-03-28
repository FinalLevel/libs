///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Http events system implementation
///////////////////////////////////////////////////////////////////////////////

#include "http_event.hpp"
#include "log.hpp"

using namespace fl::events;

const char *HttpEvent::_terminatingCharacters = "\r\n\r\n";

HttpEvent::HttpEvent(const TEventDescriptor descr, const time_t timeOutTime, HttpEventInterface *interface)
	: WorkEvent(descr, timeOutTime), _interface(interface), _networkBuffer(0), _headerStartPosition(0),
		_state(EHttpState::ST_WAIT_REQUEST), _endCharacterNumber(0), _chunkNumber(0), _status(0)
{
	setWaitRead();
}

HttpEvent::~HttpEvent()
{
	_endWork();
}

void HttpEvent::_endWork()
{
	_state = EHttpState::ST_FINISHED;
	if (_descr != 0)
		close(_descr);
	if (_networkBuffer)
	{
		auto threadSpecData = static_cast<HttpThreadSpecificData*>(_thread->threadSpecificData());
		threadSpecData->bufferPool.free(_networkBuffer);
		_networkBuffer = NULL;
	}
}

bool HttpEvent::_parseURI(const char *beginURI, const char *endURI)
{
	EHttpRequestType::EHttpRequestType requestType = EHttpRequestType::GET;
	auto firstChar = toupper(*beginURI);
	if (firstChar == 'G')	{
		requestType = EHttpRequestType::GET;
		beginURI += sizeof("GET");
	} else if (firstChar == 'P')	{
		requestType = EHttpRequestType::POST;
		beginURI += sizeof("POST");
	} else if (firstChar == 'H')	{
		requestType = EHttpRequestType::HEAD;
		beginURI += sizeof("HEAD");
	} else {
		log::Error::L("Not GET/POST/HEADER request has been received %c%c\n", firstChar, *(beginURI + 1));
		return false;
	}
	if (*(beginURI - 1) != ' ') {
		log::Error::L("Can't find an URI begin %u (%c)\n", requestType, *(beginURI - 1));
		return false;
	}
	const char *endURL =  endURI;
	while (endURL > beginURI)	{
		if (*endURL == ' ')
			break;
		endURL--;
	}
	if (endURL == beginURI) {
		log::Error::L("URL cannot be zero size %u\n", requestType);
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

	return _interface->parseURI(requestType, hostName, fileName, query);	
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
			return _interface->parseHeader(pBeginName, nameLength, pStartHeader, valueLen, pEndHeader);
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
	static const NetworkBuffer::TSize MIN_HTTP_REQUEST = sizeof("GET / HTTP/1.0\r\n\r\n") - 1;
	if (_networkBuffer->size() > MIN_HTTP_REQUEST)	{
		const char *pBuffer = _networkBuffer->c_str() + lastChecked;
		auto endCharacter = _terminatingCharacters[_endCharacterNumber];
		for ( ; lastChecked < _networkBuffer->size(); lastChecked++) {
			auto queryCharacter = *pBuffer;
			if (endCharacter == queryCharacter)
			{
				if (endCharacter == '\n') {
					const char *pEndHeader = pBuffer - 1; // skip \r\n
					if (!_headerStartPosition) { // parse URI
						if (!_parseURI(_networkBuffer->c_str(), pEndHeader))
							return false;
					}
					else if (!_parseHeader(_networkBuffer->c_str() + _headerStartPosition, pEndHeader))
						return false;
					_headerStartPosition = lastChecked + 1;
				}
				_endCharacterNumber++;
				endCharacter = _terminatingCharacters[_endCharacterNumber];
				if (!endCharacter) // \r\n\r\n
					break;
			} else if (_endCharacterNumber) {
				_endCharacterNumber = 0;
				endCharacter = _terminatingCharacters[_endCharacterNumber];
			}
			pBuffer++;
		}
		if (!endCharacter) { // end query was found
			_headerStartPosition = lastChecked + 1;
			
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

HttpEvent::ECallResult HttpEvent::_sendAnswer()
{
	auto res = _networkBuffer->send(_descr);
	if (res == NetworkBuffer::IN_PROGRESS) {
		setWaitSend();
		if (_thread->ctrl(this)) {
			return SKIP;
		}
		else
			return FINISHED;
	} else if (res == NetworkBuffer::OK) {
		if (_state != EHttpState::ST_SEND_AND_CLOSE) {
			/*if (_reset())
				return CHANGE;*/
		}
	}
	return FINISHED;	
}

HttpEvent::ECallResult HttpEvent::_sendError()
{
	_networkBuffer->clear();
	if (!_interface->formError(_state, *_networkBuffer))
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
			if (!_readPostData())
				return _sendError();
		}
		if (_state == EHttpState::ST_REQUEST_RECEIVED) {
			
		}
		else
			return SKIP;
	}
	
	if (events & E_OUTPUT) {
		if ((_state == EHttpState::ST_SEND) ||  (_state == EHttpState::ST_SEND_AND_CLOSE)) {
			return _sendAnswer();
		} else {
			log::Error::L("Output event is in error state (%u/%u)\n", _events, _state);
			return FINISHED;
		}
	}
	return SKIP;
}


