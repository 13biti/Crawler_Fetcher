#include "QueueManager.h"
#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <iostream>
#include <string>

QueueManager::QueueManager(const char *hostname, int port) {
  conn = amqp_new_connection();
  socket = amqp_tcp_socket_new(conn);

  if (!socket) {
    std::cerr << "Error creating TCP socket" << std::endl;
    exit(EXIT_FAILURE);
  }

  int status = amqp_socket_open(socket, hostname, port);
  if (status) {
    std::cerr << "Error opening TCP socket" << std::endl;
    exit(EXIT_FAILURE);
  }

  amqp_rpc_reply_t reply = amqp_login(conn, "/", 0, 131072, 0,
                                      AMQP_SASL_METHOD_PLAIN, "guest", "guest");
  if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
    std::cerr << "Error logging in" << std::endl;
    exit(EXIT_FAILURE);
  }

  amqp_channel_open(conn, 1);
  reply = amqp_get_rpc_reply(conn);
  if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
    std::cerr << "Error opening channel" << std::endl;
    exit(EXIT_FAILURE);
  }
}

QueueManager::~QueueManager() {
  amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
  amqp_destroy_connection(conn);
}

bool QueueManager::createQueue(const std::string &queueName) {
  amqp_queue_declare_ok_t *r =
      amqp_queue_declare(conn, 1, amqp_cstring_bytes(queueName.c_str()), 0, 0,
                         0, 0, amqp_empty_table);
  if (r == NULL) {
    std::cerr << "Error declaring queue" << std::endl;
    return false;
  }
  return true;
}

bool QueueManager::sendMessage(const std::string &queue_name,
                               const std::string &message) {
  // Convert message to amqp_bytes_t
  amqp_bytes_t body = amqp_cstring_bytes(message.c_str());

  // Set message properties
  amqp_basic_properties_t props;
  props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
  props.content_type = amqp_cstring_bytes("text/plain");
  props.delivery_mode = 2; // persistent delivery mode

  // Publish the message
  int status = amqp_basic_publish(conn, 1, amqp_cstring_bytes(""),
                                  amqp_cstring_bytes(queue_name.c_str()), 0, 0,
                                  &props, body);

  // Check for errors
  if (status != AMQP_STATUS_OK) {
    std::cerr << "Failed to publish message. Error: " << status << std::endl;
    return false;
  }

  std::cout << "Message sent to queue: " << queue_name << std::endl;
  return true;
}

std::string QueueManager::receiveMessage(const std::string &queue_name) {
  // Declare the queue (optional, ensures the queue exists)
  amqp_queue_declare_ok_t *declare_reply =
      amqp_queue_declare(conn, 1, amqp_cstring_bytes(queue_name.c_str()), 0, 0,
                         0, 0, amqp_empty_table);
  if (declare_reply == NULL) {
    std::cerr << "Error declaring queue: " << queue_name << std::endl;
    return "";
  }

  // Consume a single message
  amqp_basic_consume(conn, 1, amqp_cstring_bytes(queue_name.c_str()),
                     amqp_empty_bytes, 0, 1, 0, amqp_empty_table);
  amqp_rpc_reply_t consume_reply = amqp_get_rpc_reply(conn);
  if (consume_reply.reply_type != AMQP_RESPONSE_NORMAL) {
    std::cerr << "Error consuming from queue: " << queue_name << std::endl;
    return "";
  }

  // Get the message
  amqp_envelope_t envelope;
  amqp_maybe_release_buffers(conn);
  amqp_rpc_reply_t ret = amqp_consume_message(conn, &envelope, NULL, 0);

  if (ret.reply_type != AMQP_RESPONSE_NORMAL) {
    if (ret.reply_type == AMQP_RESPONSE_LIBRARY_EXCEPTION &&
        ret.library_error == AMQP_STATUS_TIMEOUT) {
      std::cout << "Queue is empty: " << queue_name << std::endl;
    } else {
      std::cerr << "Error receiving message from queue: " << queue_name
                << std::endl;
    }
    return "";
  }

  // Extract the message body
  std::string message((char *)envelope.message.body.bytes,
                      envelope.message.body.len);

  // Acknowledge the message
  amqp_basic_ack(conn, 1, envelope.delivery_tag, 0);

  // Clean up
  amqp_destroy_envelope(&envelope);

  return message;
}
amqp_connection_state_t QueueManager::getConnection() { return conn; }
