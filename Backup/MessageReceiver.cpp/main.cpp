#include <iostream>
#include <SimpleAmqpClient/SimpleAmqpClient.h>

int main() {
    const char *queueName = "PublisherQueue"; // Update this to match your queue name

    const char *host = "localhost";
    int port = 5672;
    const char *username = "guest";
    const char *password = "guest";

    AmqpClient::Channel::ptr_t channel = AmqpClient::Channel::Create(host, port, username, password);

    // Declare the exchange and queue if not already declared
    std::string exchangeName = "amq.direct";
    channel->DeclareExchange(exchangeName, AmqpClient::Channel::EXCHANGE_TYPE_DIRECT, false, true);

    // You can remove the queue declaration and binding if they are already done in another part of your code

    AmqpClient::Envelope::ptr_t envelope;
    bool success = channel->BasicConsumeMessage(queueName, envelope);

    if (success) {
        std::string message = envelope->Message()->Body();
        std::cout << "Received message: " << message << std::endl;
        // Acknowledge the message
        channel->BasicAck(envelope);
    } else {
        std::cerr << "Failed to consume message from queue." << std::endl;
    }

    return 0;
}
