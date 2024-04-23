#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include "SimplePublisher.h"
int main () {
    const char *queueName = (getenv("SENDER_AMQP_BROKER_QUEUE_NAME") != NULL) ?
            getenv("SENDER_AMQP_BROKER_QUEUE_NAME") : "PublisherQueue";
    const char *host = "localhost";
    int port = 8001;
    const char *username = "gust";
    const char *password = "gust";
    AmqpClient::Channel::ptr_t channel = AmqpClient::Channel::Create(host, port, username, password);
    std::string exchangeName = "amq.direct";
    channel->DeclareExchange(exchangeName, AmqpClient::Channel::EXCHANGE_TYPE_DIRECT);
    std::pair<std::string, int> ttl_message("x-message-ttl", 60000); // 1 minute
    std::pair<std::string, int> max_message("x-max-length", 100); // only 100 message at time in queue
    std::pair<std::string, std::string> x_overflow("x-overflow", "drop-head");
    channel->DeclareQueue(queueName, false, true, true, false, {ttl_message, max_message, x_overflow});
    channel->BindQueue(queueName , exchangeName , "");

    AmqpClient::SimplePublisher publisher(channel, exchangeName);

    // Publish a message
    std::string message = "Hello, RabbitMQ!";
    publisher.Publish(message);

    return 0;
}

