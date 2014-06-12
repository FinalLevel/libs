#pragma once
#ifndef __FL_DB_NOMOS_HPP
#define	__FL_DB_NOMOS_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Nomos storage C++ API classes
///////////////////////////////////////////////////////////////////////////////

#include <string>
#include <memory>
#include <vector>
#include <map>

#include "socket.hpp"
#include "mutex.hpp"

namespace fl {
	namespace db {
		using namespace fl::network;
		using fl::threads::Mutex;
		using fl::threads::AutoMutex;
		
		class Nomos
		{
		public:
			class ConnectionError {};
			
			Nomos(uint32_t serverId, const TIPv4 ip, TPort16 port, const uint32_t timeout);
			Nomos(const Nomos &src);
			bool get(const std::string &level, const uint64_t subLevel, const uint64_t key, const uint32_t lifeTime, 
				BString &data);
		private:
			static const std::string VERSION;
			void _connect();
			bool _receiveAnswer(BString &buf);
			uint32_t _serverId;
			TIPv4 _ip;
			TPort16 _port;
			uint32_t _timeout;
			Socket _conn;
		};
		
		class NomosPool
		{
		public:
			class Prefered
			{
			public:
				Prefered(const uint32_t mainId, const uint32_t backupId)
					: mainId(mainId), backupId(backupId)
				{
				}
			private:
				friend class NomosPool;
				uint32_t mainId;
				uint32_t backupId;
			};
			NomosPool(const size_t maxConnectionsPerServer, const uint32_t timeout);
			~NomosPool();
			void addServer(const uint32_t serverId, const TIPv4 ip, const TPort16 port);
			bool get(const std::string &level, const uint64_t subLevel, const uint64_t key, const uint32_t lifeTime, 
				BString &data, const Prefered &prefered);
		private:
			size_t _maxConnectionsPerServer;
			size_t _timeout;
			
			typedef std::shared_ptr<Nomos> TNomosPtr;
			typedef std::vector<TNomosPtr> TNomosPtrVector;
			struct Server
			{
				Server(const uint32_t serverId, const TIPv4 ip, const TPort16 port, const uint32_t timeout)
				{
					_servers.push_back(TNomosPtr(new Nomos(serverId, ip, port, timeout)));
				}
				TNomosPtr get(size_t maxConnectionsPerServer);
				TNomosPtrVector _servers;
			};
			typedef std::map<uint32_t, Server*> TNomosVectorMap;
			TNomosVectorMap _servers;
			Mutex _sync;
			
			bool _findServers(const Prefered &prefered, TNomosPtrVector &servers);
		};
	};
};

#endif	// __FL_DB_NOMOS_HPP
