#include <cstdlib>
#include <iostream>
#include <string>

class MessageHandler {
public:
  static void sendMessage(const std::string &queue_name,
                          const std::string &message) {
    std::string command = "python3 sender.py " + queue_name + " " + message;
    int result = system(command.c_str());
    if (result != 0) {
      std::cerr << "Failed to send message using Python script" << std::endl;
    }
  }

  static std::string receiveMessage(const std::string &queue_name) {
    std::string command = "python3 receiver.py " + queue_name;
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
      std::cerr << "Failed to call Python receiver script" << std::endl;
      return "";
    }

    char buffer[128];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
      result += buffer;
    }
    pclose(pipe);

    return result;
  }
};

int main() {
  // Example usage
  MessageHandler::sendMessage("test_queue", "Hello, RabbitMQ!");

  std::string received_message = MessageHandler::receiveMessage("test_queue");
  std::cout << "Received message: " << received_message << std::endl;

  return 0;
}
