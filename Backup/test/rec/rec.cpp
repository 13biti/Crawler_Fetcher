#include <iostream>
#include <stdlib.h>
#include <SimpleAmqpClient/SimpleAmqpClient.h>

int main() {
    const char* queueName = getenv("RECEIVER_AMQP_BROKER_QUEUE_NAME") != NULL ?
                             getenv("RECEIVER_AMQP_BROKER_QUEUE_NAME") : "SubscriberQueue";

    const char* host = "localhost";
    int port = 5672;
    const char* username = "guest";
    const char* password = "guest";

    AmqpClient::Channel::ptr_t channel = AmqpClient::Channel::Create(host, port, username, password);
    channel->DeclareQueue(queueName, false, true, false, true);

    std::string consumerTag = "my_consumer_tag"; // Set a unique consumer tag

    while (true) {
        AmqpClient::Envelope::ptr_t envelope;
        bool success = false;
        try {
            channel->BasicConsume(queueName, consumerTag, true, false);
            success = channel->BasicConsumeMessage(consumerTag, envelope, 0);
        } catch (const std::exception& e) {
            std::cerr << "Exception caught: " << e.what() << std::endl;
        }

        if (success) {
            std::string message = envelope->Message()->Body();
            std::cout << "Received message: " << message << std::endl;
            break; // Exit the loop after receiving one message
        } else {
            std::cerr << "Failed to consume message from queue." << std::endl;
            break; // Exit the loop if message consumption fails
        }
    }

    return 0;
}

