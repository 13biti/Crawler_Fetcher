// MessageHandler.cpp
#include "messageManagerClass.h"

MessageHandler::MessageHandler(const std::string& host, int port,
                               const std::string& username, const std::string& password ,
                               const std::string& receiverQueueName ,
                               const std::string& senderQueueName)
    : m_host(host), m_port(port),
    m_username(username), m_password(password),
    m_receiverQueue(receiverQueueName) , m_senderQueue(senderQueueName) {}

void MessageHandler::sendMessage(const std::string& message) {
    try {
        auto channel = AmqpClient::Channel::Create(m_host, m_port, m_username, m_password);
        channel->DeclareQueue(m_senderQueue, false, true, false, false);
        auto msg = AmqpClient::BasicMessage::Create(message);
        channel->BasicPublish("", m_senderQueue, msg);
        std::clog << " [x] Sent '" << message << "'" << std::endl;
    } catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
    }
}

std::string MessageHandler::receiveMessage() {
    try {
        auto channel = AmqpClient::Channel::Create(m_host, m_port, m_username, m_password);
        channel->DeclareQueue(m_receiverQueue, false, true, false,false);
        auto consumerTag = channel->BasicConsume(m_receiverQueue);
        std::clog << "Consumer tag: " << consumerTag << std::endl;
        auto envelope = channel->BasicConsumeMessage(consumerTag);
        std::clog << " [x] Received " << envelope->Message()->Body() << std::endl;
        channel->BasicAck(envelope);
        return envelope->Message()->Body();
    } catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
    }
}
void MessageHandler::receiveMessage2() {
    try {
        auto channel = AmqpClient::Channel::Create(m_host, m_port, m_username, m_password);
        channel->DeclareQueue(m_receiverQueue, false, true, false, false);
        auto consumerTag = channel->BasicConsume(m_receiverQueue);
        std::clog << "Consumer tag: " << consumerTag << std::endl;

        auto envelope = channel->BasicConsumeMessage(consumerTag);
        if (envelope) {
            std::clog << " [x] Received " << envelope->Message()->Body() << std::endl;
            // Acknowledge the message to remove it from the queue
            channel->BasicAck(envelope);
        } else {
            std::clog << "No message available in the queue." << std::endl;
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
    }
}

