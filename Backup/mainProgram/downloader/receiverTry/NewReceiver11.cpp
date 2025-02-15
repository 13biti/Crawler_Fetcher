#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <iostream>
#include <string>
int main() {
  // Connection and channel setup
  amqp::Connection connection("amqp://localhost:5672", "guest", "guest"); // Replace with your credentials
  amqp::Channel channel(&connection);

  // Declare the queue (if it doesn't exist already)
  channel.queue_declare("test", amqp::durable);

  // Basic consumer setup (non-auto acknowledge)
  amqp::BasicConsume consumer(channel);
  consumer.queue("test");
  consumer.no_ack(true); // Disable automatic acknowledgement

  // Get a single message (non-blocking)
  amqp::Envelope envelope = consumer.get();

  // Check if a message was received
  if (envelope) {
    const std::string& message_body = envelope.message();
    std::cout << "Received message: " << message_body << std::endl;

    // Acknowledge the message (remove from queue)
    channel.basic_ack(envelope.delivery_tag(), false);
  } else {
    std::cout << "No message received." << std::endl;
  }

  // Close the connection
  connection.close();
  return 0;
}

