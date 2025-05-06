#include "../include/QueueManager.h"
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

int main() {
  // Configuration
  const std::string BASE_URL = "http://127.0.0.1:5000";
  const std::string WRITE_USERNAME = "u2";
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

  std::vector<std::string> messages = {
      "www.google.com",
      "www.microsoft.com",
      "www.github.com",
      "www.stackoverflow.com",
      "www.wikipedia.org",
      "www.reddit.com",
      "www.linkedin.com",
      "www.twitter.com",
      "www.facebook.com",
      "www.instagram.com",
      "www.netflix.com",
      "www.amazon.com",
      "www.nytimes.com",
      "www.bbc.com",
      "www.cnn.com",
      "www.nasa.gov",
      "www.ibm.com",
      "www.apple.com",
      "www.tesla.com",
      "www.samsung.com",

      "www.wikipedia.org/wiki/Artificial_intelligence",
      "www.wikipedia.org/wiki/Quantum_mechanics",
      "www.github.com/explore",
      "www.github.com/trending",
      "www.nasa.gov/missions",
      "www.nasa.gov/news"};

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
