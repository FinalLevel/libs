///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Nomos storage C++ API classes implementation
///////////////////////////////////////////////////////////////////////////////

#include "nomos.hpp"
#include "log.hpp"
#include "mysql.hpp"

using namespace fl::db;

const std::string Nomos::VERSION("V01");

Nomos::Nomos(uint32_t serverId, const TIPv4 ip, TPort16 port, const uint32_t timeout)
	: _serverId(serverId), _ip(ip), _port(port), _timeout(timeout), _conn(INVALID_SOCKET)
{
}

Nomos::Nomos(const Nomos &src)
	: _serverId(src._serverId), _ip(src._ip), _port(src._port), _timeout(src._timeout), _conn(INVALID_SOCKET)
{
}


bool Nomos::get(const std::string &level, const uint64_t subLevel, const uint64_t key, const uint32_t lifeTime,
	BString &data)
{
	_connect();
	
	BString req;
	req.sprintfSet("%s,G,%s,%llx,%llx,%u\n", VERSION.c_str(), level.c_str(), subLevel, key, lifeTime);
	if (!_conn.pollAndSendAll(req.c_str(), req.size(), _timeout)) {
		log::Error::L("Nomos:get: can't send to %s:%u\n",  Socket::ip2String(_ip).c_str(), _port);
		_conn.reset(INVALID_SOCKET);
		throw ConnectionError();
	}
	if (!_receiveAnswer(req))
		return false;
	size_t length = strtoul(req.c_str() + 2, NULL, 16);
	data.clear();
	if (length > 0) {
		if (!_conn.pollAndRecvAll(data.reserveBuffer(length), length, _timeout)) {
			log::Error::L("Nomos:get: can't read %u of data from %s:%u\n", length, Socket::ip2String(_ip).c_str(), _port);
			_conn.reset(INVALID_SOCKET);
			throw ConnectionError();
		}
	}
	return true;
}

bool Nomos::_receiveAnswer(BString &buf)
{
	//An answer is always 11 bytes OK00000000\n
	static const size_t ANSWER_SIZE = 11;
	buf.clear();
	if (!_conn.pollAndRecvAll(buf.reserveBuffer(ANSWER_SIZE), ANSWER_SIZE, _timeout)) {
		log::Error::L("Nomos: can't receive answer from %s:%u\n",  Socket::ip2String(_ip).c_str(), _port);
		_conn.reset(INVALID_SOCKET);
		throw ConnectionError();
	}
	const char *pAnswer = buf.c_str();
	if (*pAnswer == 'E') {
		static const std::string ERR_CR("ERR_CR");
		if (!strncmp(pAnswer, "ERR_CR", ERR_CR.size())) {
			log::Error::L("Nomos: Critical error %s has been received from %s:%u\n",  Socket::ip2String(_ip).c_str(), _port,
				pAnswer);
			_conn.reset(INVALID_SOCKET);
			throw ConnectionError();
		}
		return false;
	} else {
		return true;
	}
}

void Nomos::_connect()
{
	if (_conn.descr() != INVALID_SOCKET)
		return;
	_conn.reopen();
	if (!_conn.connect(_ip, _port, _timeout)) {
		_conn.reset(INVALID_SOCKET);
		log::Error::L("Nomos: can't connect %s:%u\n",  Socket::ip2String(_ip).c_str(), _port);
		throw ConnectionError();
	}
}

NomosPool::NomosPool(const size_t maxConnectionsPerServer, const uint32_t timeout)
	: _maxConnectionsPerServer(maxConnectionsPerServer), _timeout(timeout)
{
	
}

NomosPool::~NomosPool()
{
	for (auto s = _servers.begin(); s != _servers.end(); s++)
		delete s->second;
}

void NomosPool::addServer(const uint32_t serverId, const TIPv4 ip, const TPort16 port)
{
	AutoMutex autoSync(&_sync);
	auto res = _servers.insert(TNomosVectorMap::value_type(serverId, NULL));
	if (!res.second) {
		delete res.first->second;
	}
	res.first->second = new Server(serverId, ip, port, _timeout);
}

NomosPool::TNomosPtr NomosPool::Server::get(size_t maxConnectionsPerServer)
{
	for (auto s = _servers.begin(); s != _servers.end(); s++) {
		if (s->unique()) {
			return (*s);
		}
	}
	if (_servers.size() < maxConnectionsPerServer) {
		_servers.push_back(TNomosPtr(new Nomos(*_servers.back().get())));
		return _servers.back();
	} else {
		return TNomosPtr();
	}
}

bool NomosPool::_findServers(const Prefered &prefered, TNomosPtrVector &servers)
{
	auto f = _servers.find(prefered.mainId);
	if (f != _servers.end()) {
		auto server = f->second->get(_maxConnectionsPerServer);
		if (server.get())
			servers.push_back(server);
	} 
	f = _servers.find(prefered.backupId);
	if (f != _servers.end()) {
		auto server = f->second->get(_maxConnectionsPerServer);
		if (server.get())
			servers.push_back(server);
	} 
	return !servers.empty();
}

bool NomosPool::get(const std::string &level, const uint64_t subLevel, const uint64_t key, const uint32_t lifeTime, 
	BString &data, const Prefered &prefered)
{
	TNomosPtrVector servers;
	AutoMutex autoSync(&_sync);
	if (!_findServers(prefered, servers))
		return false;
	autoSync.unLock();
	
	for (auto s = servers.begin(); s != servers.end(); s++) {
		try 
		{
			if ((*s)->get(level, subLevel, key, lifeTime, data))
				return true;
		}
		catch (Nomos::ConnectionError &ce)
		{
			
		}
	}
	return false;
	
}

