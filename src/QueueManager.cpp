#include "include/QueueManager.h"
#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <iostream>
#include <string>

QueueManager::QueueManager(const char *hostname, int port) {
  // in here , i want to login into queue manager service , try to get token and
  // store it i also need to manage the token , properly handle queue manager
  // sservice responces , and both send and recive message still have no idea on
  // how to return the responce this constractor will only get token , create
  // and handle the queue will apply in another method
}

QueueManager::~QueueManager() {
  amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
  amqp_destroy_connection(conn);
}

bool QueueManager::createQueue(const std::string &queueName) {}

bool QueueManager::sendMessage(const std::string &queue_name)
}

std::string QueueManager::receiveMessage(const std::string &queue_name) {}
