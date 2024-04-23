#include <amqpcpp.h>
#include <amqpcpp/libboostasio.h>
#include <boost/asio.hpp>

int main() {
    // Establish connection to RabbitMQ server
    auto handler = make_handler("amqp://guest:guest@localhost/");
    AMQP::TcpConnection connection(&handler, AMQP::Address("localhost", 5672));

    // Open a channel
    AMQP::TcpChannel channel(&connection);

    // Declare the queue
    channel.declareQueue("test");

    // Consume a single message from the queue
    channel.consume("test")
        .onReceived([](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {
            std::cout << "Received message: " << message.body() << std::endl;

            // Acknowledge the message
            channel.ack(deliveryTag);
        });

    // Start the event loop
    handler.loop();

    return 0;
}

