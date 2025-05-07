// this code is writed by chatgpd , i realy not write any test till now
#include "../include/QueueManager.h"
#include <gtest/gtest.h>
#include <iostream>

// Test Configuration
const std::string BASE_URL = "http://127.0.0.1:5000";
const std::string WRITE_USERNAME = "u2";
const std::string READ_USERNAME = "u1";
const std::string PASSWORD = "123";
const std::string API_LOGIN = "login";
const std::string API_SEND = "write";
const std::string API_RECEIVE = "read";
const std::string QUEUE_NAME = "test_queue";

// Test fixture for QueueManager
class QueueManagerTest : public ::testing::Test {
protected:
  QueueManager *queueManager;

  void SetUp() override { queueManager = new QueueManager(BASE_URL); }

  void TearDown() override { delete queueManager; }
};

// Test: Get token
TEST_F(QueueManagerTest, GetToken) {
  std::string token =
      queueManager->getToken(WRITE_USERNAME, PASSWORD, API_LOGIN);
  EXPECT_FALSE(token.empty()) << "Token should not be empty!";
}

// Test: Send and receive a message
TEST_F(QueueManagerTest, SendMessage) {

  std::string write_token =
      queueManager->getToken(WRITE_USERNAME, PASSWORD, API_LOGIN);
  ASSERT_FALSE(write_token.empty()) << "Failed to retrieve token!";
  // Step 2: Send Message
  std::string message = "Hello, Queue!";
  bool sendSuccess =
      queueManager->sendMessage(QUEUE_NAME, message, write_token, API_SEND);
  EXPECT_TRUE(sendSuccess) << "Failed to send message!";

  sendSuccess =
      queueManager->sendMessage(QUEUE_NAME, message, write_token, API_SEND);
  EXPECT_TRUE(sendSuccess) << "Failed to send message!";
}

TEST_F(QueueManagerTest, ReceiveMessage) {
  std::string read_token =
      queueManager->getToken(READ_USERNAME, PASSWORD, API_LOGIN);
  ASSERT_FALSE(read_token.empty()) << "Failed to retrieve token!";

  std::string message = "Hello, Queue!";

  // Step 3: Receive Message
  std::string receivedMessage =
      queueManager->receiveMessage(QUEUE_NAME, read_token, API_RECEIVE);
  EXPECT_EQ(receivedMessage, message)
      << "Received message does not match sent message!";
}
// Main function for running tests
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
