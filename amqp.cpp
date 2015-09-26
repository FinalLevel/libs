///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: AMQP protocol wrapper class implementation
///////////////////////////////////////////////////////////////////////////////

#include "amqp.hpp"	
#include "log.hpp"

using namespace fl::amqp;
using namespace fl;
using fl::strings::BString;

AmqpConnection::AmqpConnection()
	: _conn(amqp_new_connection()), _lastChannelId(1)
{
	_socket = amqp_tcp_socket_new(_conn);
}

AmqpConnection::~AmqpConnection()
{
	amqp_connection_close(_conn, AMQP_REPLY_SUCCESS);
	amqp_destroy_connection(_conn);
}

bool checkResult(amqp_rpc_reply_t x, char const *context)
{
  switch (x.reply_type) {
  case AMQP_RESPONSE_NORMAL:
    return true;

  case AMQP_RESPONSE_NONE:
    log::Error::L("%s: missing RPC reply type!\n", context);
    break;

  case AMQP_RESPONSE_LIBRARY_EXCEPTION:
    log::Error::L("%s: %s\n", context, amqp_error_string2(x.library_error));
    break;

  case AMQP_RESPONSE_SERVER_EXCEPTION:
    switch (x.reply.id) {
    case AMQP_CONNECTION_CLOSE_METHOD: {
      amqp_connection_close_t *m = (amqp_connection_close_t *) x.reply.decoded;
      log::Error::L("%s: server connection error %d, message: %.*s\n",
              context,
              m->reply_code,
              (int) m->reply_text.len, (char *) m->reply_text.bytes);
      break;
    }
    case AMQP_CHANNEL_CLOSE_METHOD: {
      amqp_channel_close_t *m = (amqp_channel_close_t *) x.reply.decoded;
      log::Error::L("%s: server channel error %d, message: %.*s\n",
              context,
              m->reply_code,
              (int) m->reply_text.len, (char *) m->reply_text.bytes);
      break;
    }
    default:
      log::Error::L("%s: unknown server error, method id 0x%08X\n", context, x.reply.id);
      break;
    }
    break;
  }
	return false;
}

bool AmqpConnection::plainLogin(const std::string &hostname, const uint32_t port, const std::string &user, 
	const std::string &password, const std::string &virtualHost)
{
	if (amqp_socket_open(_socket, hostname.c_str(), port)) {
		return checkResult(amqp_get_rpc_reply(_conn), "Opening socket");
	}
	return checkResult(amqp_login(_conn, virtualHost.c_str(), 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, user.c_str(), 
		password.c_str()), "logging");
}

TChannelPtr AmqpConnection::openChannel() 
{
	if (!amqp_channel_open(_conn, _lastChannelId)) {
		return TChannelPtr();
	}
	if (!checkResult(amqp_get_rpc_reply(_conn), "Opening socket")) {
		return TChannelPtr();
	}
	TChannelPtr channel(new Channel(_lastChannelId, _conn));
	_lastChannelId++;
	return channel;
}

Channel::Channel(const amqp_channel_t channelId, amqp_connection_state_t conn)
	: _channelId(channelId), _conn(conn)
{
}

bool Channel::declareQueue(const std::string &name, const bool passive, const bool durable, const bool exclusive, 
	const bool autoDelete)
{
	amqp_queue_declare(_conn, _channelId, amqp_cstring_bytes(name.c_str()), passive, durable, 
		exclusive, autoDelete, amqp_empty_table);
	return checkResult(amqp_get_rpc_reply(_conn), "Declaring queue");
}

bool Channel::bindQueue(const std::string &name, const std::string &exchange, const std::string &routingKey)
{
	amqp_queue_bind(_conn, _channelId, amqp_cstring_bytes(name.c_str()), amqp_cstring_bytes(exchange.c_str()), 
		amqp_cstring_bytes(routingKey.c_str()), amqp_empty_table);
	return checkResult(amqp_get_rpc_reply(_conn), "Binding queue");
}

bool Channel::basicConsume(const std::string &queueName)
{
	amqp_basic_consume(_conn, _channelId, amqp_cstring_bytes(queueName.c_str()), amqp_empty_bytes, 0, 1, 0, 
		amqp_empty_table);
	return checkResult(amqp_get_rpc_reply(_conn), "Consuming");
}

bool Channel::get(fl::strings::BString &data) 
{
	amqp_envelope_t envelope;
	amqp_maybe_release_buffers(_conn);
	auto res = amqp_consume_message(_conn, &envelope, NULL, 0);

	if (AMQP_RESPONSE_NORMAL != res.reply_type) {
		return false;
	}
	data.clear();
	data.add(static_cast<char*>(envelope.message.body.bytes), envelope.message.body.len);
	amqp_destroy_envelope(&envelope);
	return true;
}

bool Channel::basicPublish(const std::string &exchange, const std::string &routingKey, const char *contentType,
	const bool mandatory, const bool immediate, const char *body)
{
	amqp_basic_properties_t props;
	props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
	props.content_type = amqp_cstring_bytes(contentType);
	props.delivery_mode = 2; /* persistent delivery mode */
	if (amqp_basic_publish(_conn, _channelId,
		amqp_cstring_bytes(exchange.c_str()),
		amqp_cstring_bytes(routingKey.c_str()),
		mandatory,
		immediate,
		&props,
		amqp_cstring_bytes(body)) > 0) {
			return checkResult(amqp_get_rpc_reply(_conn), "Publishing event");
	} else {
		return true;
	}
}

Channel::~Channel()
{
	checkResult(amqp_channel_close(_conn, _channelId, AMQP_REPLY_SUCCESS), "Closing channel");
}