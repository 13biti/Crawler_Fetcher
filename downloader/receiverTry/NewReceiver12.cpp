#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <amqp_framing.h>
#include <cstring> // for std::strcmp

int main() {
  // Connection setup
  amqp_connection_t connection = amqp_tcp_socket_new("localhost:5672"); // Replace with your credentials

  if (!connection) {
    std::cerr << "Error: Could not create AMQP connection." << std::endl;
    return 1;
  }

  // Channel creation
  amqp_channel_t channel = amqp_channel_new(connection);

  if (!channel) {
    amqp_connection_close(connection, AMQP_ERROR_INTERNAL);
    amqp_connection_destroy(connection);
    std::cerr << "Error: Could not create AMQP channel." << std::endl;
    return 1;
  }

  // Queue declaration (if it doesn't exist already)
  amqp_basic_declare_ok_t queue_result = amqp_basic_declare(channel,
                                                          /* queue name */ "test",
                                                          /* passive */ 0,
                                                          /* durable */ 0,
                                                          /* exclusive */ 0,
                                                          /* auto_delete */ 0,
                                                          /* arguments */ NULL);

  if (queue_result.reply_code != AMQP_REPLY_OK) {
    std::cerr << "Error: Queue declaration failed." << std::endl;
    amqp_channel_close(channel, AMQP_ERROR_INTERNAL);
    amqp_channel_destroy(channel);
    amqp_connection_close(connection, AMQP_ERROR_INTERNAL);
    amqp_connection_destroy(connection);
    return 1;
  }

  // Basic consumer setup (non-auto acknowledge)
  amqp_basic_consume_ok_t consumer_result = amqp_basic_consume(channel,
                                                              /* queue */ "test",
                                                              /* consumer tag */ NULL,
                                                              /* no local */ 0,
                                                              /* no ack */ 1,
                                                              /* exclusive */ 0,
                                                              /* arguments */ NULL);

  if (consumer_result.reply_code != AMQP_REPLY_OK) {
    std::cerr << "Error: Basic consumer creation failed." << std::endl;
    amqp_channel_close(channel, AMQP_ERROR_INTERNAL);
    amqp_channel_destroy(channel);
    amqp_connection_close(connection, AMQP_ERROR_INTERNAL);
    amqp_connection_destroy(connection);
    return 1;
  }

  // Get a single message (blocking)
  amqp_basic_deliver_t delivery;
  amqp_frame_t frame;

  amqp_rpc_reply_t ret = amqp_basic_get(channel, &consumer_result.consumer_tag, 1);
  if (ret.reply_code == AMQP_REPLY_NOT_FOUND) {
    std::cout << "No message received." << std::endl;
  } else if (ret.reply_code != AMQP_REPLY_OK) {
    std::cerr << "Error: Get message failed." << std::endl;
    amqp_channel_close(channel, AMQP_ERROR_INTERNAL);
    amqp_channel_destroy(channel);
    amqp_connection_close(connection, AMQP_ERROR_INTERNAL);
    amqp_connection_destroy(connection);
    return 1;
  } else {
    // Parse the delivery frame
    amqp_frame_t_init(&frame);
    amqp_decode(ret.reply.frames, ret.reply.len, &frame, 0);

    if (frame.frame_type == AMQP_FRAME_TYPE_DELIVERY) {
      delivery = frame.delivery

