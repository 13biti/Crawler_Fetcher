#include <iostream>
#include <amqpcpp.h>
#include "conn_handler.h"

int
main(void)
{
    ConnHandler handler;
    AMQP::TcpConnection connection(handler, AMQP::Address("amqp://localhost/"));
    AMQP::TcpChannel channel(&connection);
    channel.onError([&handler](const char* message)
        {
            std::cout << "Channel error: " << message << std::endl;
            handler.Stop();
        });
    channel.declareQueue("test", AMQP::autodelete)
        .onSuccess
        (
            [&connection](const std::string &name,
                          uint32_t messagecount,
                          uint32_t consumercount)
            {
                std::cout << "Created queue: " << name << std::endl;
            }
        );
    channel.consume("test", AMQP::noack)
        .onReceived
        (
            [](const AMQP::Message &msg, uint64_t tag, bool redelivered)
            {
                std::cout << "Received: " << msg<< std::endl;
            }
        );
    handler.Start();
    std::cout << "Closing connection." << std::endl;
    connection.close();
    return 0;
}


