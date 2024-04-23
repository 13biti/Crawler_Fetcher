#include <iostream>
#include <amqpcpp.h>
#include <amqpcpp/libboostasio.h>
#include <boost/asio.hpp>

int main() {
    // Establish connection to RabbitMQ server
    AMQP::Address address("localhost", 5672, AMQP::Login("guest", "guest"), "/");
    AMQP::TcpConnection connection(&address);

    // Open a channel
    AMQP::TcpChannel channel(&connection);

    // Declare the queue
    channel.declareQueue("test");

    // Consume a single message from the queue
    channel.consume("test")
        .onReceived([&channel](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {
            std::cout << "Received message: " << message.body() << std::endl;

            // Acknowledge the message
            channel.ack(deliveryTag);
        });

    // Start the event loop
    AMQP::TcpHandler handler(&connection);
    handler.loop();

    return 0;
}

