#ifndef QUEUE_MANAGER_H
#define QUEUE_MANAGER_H

#include <amqp.h>
#include <amqp_framing.h>
#include <amqp_tcp_socket.h>
#include <iostream>
#include <string>

class QueueManager {
private:
  std::string Active_Token;
  std::string Queue_Manager_Server_Base_Url;
  void setActive_token(const std::string &token) { Active_Token = token; }
  void unsetActive_token() { Active_Token = ""; }

public:
  QueueManager(const std::string &queue_manager_base_url)
      : Queue_Manager_Server_Base_Url(queue_manager_base_url) {};

  // this method will only use to get token , it will not create queue !!!
  // destructor!!!
  ~QueueManager();
  const std::string &getToken(const std::string &username,
                              const std::string &password,
                              const std::string &api);
  void removeToken();
  bool createQueue(const std::string &queueName, std::string &api);
  bool sendMessage(const std::string &queue_name, const std::string &message,
                   const std::string &token, std::string api);
  bool sendMessage(const std::string &queue_name, const std::string &message,
                   std::string api) {
    return sendMessage(queue_name, message, Active_Token, api);
  };

  std::string receiveMessage(const std::string &queue_name,
                             const std::string &token,
                             std::string api = "read");

  std::string receiveMessage(const std::string &queue_name,
                             std::string api = "read") {
    return receiveMessage(queue_name, Active_Token, api);
  };
};
#endif
