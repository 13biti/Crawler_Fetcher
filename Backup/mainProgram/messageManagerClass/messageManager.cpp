#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include <iostream>

constexpr auto SENDER_QUEUE_NAME = "senderQueue";
constexpr auto RECEIVER_QUEUE_NAME = "senderQueue";

class MessageManager {
public:
    MessageManager(const std::string& host, int port,
                   const std::string& username, const std::string& password)
        : m_host(host), m_port(port), m_username(username), m_password(password) {}

    void sendMessage(const std::string& message) {
        try {
            auto channel = AmqpClient::Channel::Create(m_host, m_port, m_username, m_password);
            channel->DeclareQueue(SENDER_QUEUE_NAME, false, true, false, true);
            auto msg = AmqpClient::BasicMessage::Create(message);
            channel->BasicPublish("", SENDER_QUEUE_NAME, msg);
            std::clog << " [x] Sent '" << message << "'" << std::endl;
        } catch (const std::exception& error) {
            std::cerr << error.what() << std::endl;
        }
    }

    void receiveMessage() {
        try {
            auto channel = AmqpClient::Channel::Create(m_host, m_port, m_username, m_password);
            channel->DeclareQueue(RECEIVER_QUEUE_NAME, false, true, false, true);
            auto consumerTag = channel->BasicConsume(RECEIVER_QUEUE_NAME);
            std::clog << "Consumer tag: " << consumerTag << std::endl;
            auto envelope = channel->BasicConsumeMessage(consumerTag);
            std::clog << " [x] Received " << envelope->Message()->Body() << std::endl;
        } catch (const std::exception& error) {
            std::cerr << error.what() << std::endl;
        }
    }

private:
    std::string m_host;
    int m_port;
    std::string m_username;
    std::string m_password;
};

int main() {
    const char* host = "localhost";
    int port = 5672;
    const char* username = "guest";
    const char* password = "guest";

    MessageManager manager(host, port, username, password);
    manager.sendMessage("Hello World!");

    manager.receiveMessage();

    return 0;
}

