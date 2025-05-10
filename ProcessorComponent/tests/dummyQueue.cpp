#include "../../Common/QueueManager/QueueManager.h"
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

int main() {
  // Configuration
  const std::string BASE_URL = "http://127.0.0.1:5000";
  const std::string WRITE_USERNAME = "u3";
  const std::string READ_USERNAME = "u1";
  const std::string PASSWORD = "123";
  const std::string API_LOGIN = "login";
  const std::string API_SEND = "write";
  const std::string API_RECEIVE = "read";
  const std::string QUEUE_NAME = "rawLinks";

  QueueManager queueManager(BASE_URL);
  QueueManager queueManagerreader(BASE_URL);

  std::string token =
      queueManager.getToken(WRITE_USERNAME, PASSWORD, API_LOGIN);
  std::string tokenread =
      queueManager.getToken(READ_USERNAME, PASSWORD, API_LOGIN);
  if (token.empty()) {
    std::cerr << "Failed to retrieve token!" << std::endl;
    return 1;
  }

  std::vector<std::string> messages = {"www.google.com", "znu.ac.ir",
                                       "news.znu.ac.ir", "food.znu.ac.ir",
                                       "cert.znu.ac.ir", "www.nasa.gov/news"};

  for (const auto &msg : messages) {
    bool success = queueManager.sendMessage(QUEUE_NAME, msg, token, API_SEND);
    if (!success) {
      std::cerr << "Failed to send: " << msg << std::endl;
    } else {
      std::cout << "Sent: " << msg << std::endl;
    }
  }

  return 0;
}
