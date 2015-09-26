#pragma once
#ifndef __FL_AMQP_
#define __FL_AMQP_

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: AMQP protocol wrapper class
///////////////////////////////////////////////////////////////////////////////

#include <string>
#include <memory>
#include <amqp_tcp_socket.h>
#include <amqp_framing.h>
#include <amqp.h>

#include "bstring.hpp"

namespace fl {
	namespace amqp {
		class Channel
		{
		public:
			Channel(const amqp_channel_t channelId, amqp_connection_state_t conn);
			~Channel();
			bool declareQueue(const std::string &name, const bool passive, const bool durable, const bool exclusive, 
				const bool autoDelete);
			bool bindQueue(const std::string &queueName, const std::string &exchange, const std::string &routingKey);
			bool basicPublish(const std::string &exchange, const std::string &routingKey, const char *contentType,
				const bool mandatory, const bool immediate, const char *body);
			bool basicConsume(const std::string &queueName);
			bool get(fl::strings::BString &data);
		private:
			amqp_channel_t _channelId;
			amqp_connection_state_t _conn;
		};
		
		using TChannelPtr = std::unique_ptr<Channel>;
		
		class AmqpConnection 
		{
		public:
			AmqpConnection();
			~AmqpConnection();
			bool plainLogin(const std::string &hostname, const uint32_t port, const std::string &user, 
				const std::string &password, const std::string &virtualHost);
			TChannelPtr openChannel();
		private:
			amqp_socket_t *_socket;
			amqp_connection_state_t _conn;
			uint32_t _lastChannelId;
		};
	};
};

#endif // __FL_AMQP_
