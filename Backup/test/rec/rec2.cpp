#include <iostream>
#include <SimpleAmqpClient/SimpleAmqpClient.h>

int main() {
    const char* queueName = "PublisherQueue";
    const char* host = "localhost";
    int port = 5672;
    const char* username = "guest";
    const char* password = "guest";

    // Create a connection to the RabbitMQ server
    AmqpClient::Channel::ptr_t channel = AmqpClient::Channel::Create(host, port, username, password);

    // Declare the queue
    channel->DeclareQueue(queueName, false, true, true, false);

    // Start consuming messages from the queue
    std::string consumerTag = "simple_consumer";
    channel->BasicConsume(queueName, consumerTag, true, false);

    while (true) {
        // Wait for a message
        AmqpClient::Envelope::ptr_t envelope;
        bool success = channel->BasicConsumeMessage(consumerTag, envelope, 0);
        if (success) {
            // Extract message body
            std::string message = envelope->Message()->Body();
            std::cout << "Received message: " << message << std::endl;
        } else {
            std::cerr << "Failed to consume message from queue." << std::endl;
        }
    }

    return 0;
}

