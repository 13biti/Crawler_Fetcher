#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include "simpleSender.h"
int main () {
    const char *queueName = (getenv("SENDER_AMQP_BROKER_QUEUE_NAME") != NULL) ?
                            getenv("SENDER_AMQP_BROKER_QUEUE_NAME") : "PublisherQueue";
    std::cout<<queueName<<std::endl;
    const char *host = "localhost";
    int port = 5672;
    const char *username = "guest";
    const char *password = "guest";
    AmqpClient::Channel::ptr_t channel = AmqpClient::Channel::Create(host, port, username, password);
    std::string exchangeName = "amq.direct";
    channel->DeclareExchange(exchangeName, AmqpClient::Channel::EXCHANGE_TYPE_DIRECT , false , true);
    std::pair<std::string, int> ttl_message("x-message-ttl", 60000); // 1 minute
    std::pair<std::string, int> max_message("x-max-length", 100); // only 100 message at time in queue
    std::pair<std::string, std::string> x_overflow("x-overflow", "drop-head");
    channel->DeclareQueue(queueName, false, true, false, true, {ttl_message, max_message, x_overflow});
    channel->BindQueue(queueName , exchangeName , "");

    AmqpClient::SimplePublisher publisher(channel, exchangeName);

    // Publish a message
    std::string message = "Hello, RabbitMQ!";
    publisher.Publish(message);
    for (int i = 0 ; i <200 ; i++){
        message.append("s");
        publisher.Publish(message);
    }
    int i = 0 ;
    while(1){
        i ++;
        i --;
    }
    return 0;
}

