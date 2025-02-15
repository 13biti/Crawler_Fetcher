#ifndef QUEUE_MANAGER_H
#define QUEUE_MANAGER_H

#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>
#include <iostream>
#include <string>

class QueueManager {
private:
    amqp_connection_state_t conn;
    amqp_socket_t *socket = NULL;
    amqp_connection_state_t getConnection();

public:
    QueueManager(const char* hostname, int port);
    ~QueueManager();

    bool createQueue(const std::string& queueName);
    bool sendMessage(const std::string& queue_name, const std::string& message);
    std::string receiveMessage(const std::string& queue_name);
};

#endif

