// MessageHandler.h
#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include <iostream>

class MessageHandler {
public:
    MessageHandler(const std::string& host, int port,
                   const std::string& username, const std::string& password,
                   const std::string& receiverQueueName , const std::string& senderQueueName);

    void sendMessage(const std::string& message);
    void receiveMessage();
    void receiveMessage2();

private:
    std::string m_host;
    int m_port;
    std::string m_username;
    std::string m_password;
    std::string m_senderQueue;
    std::string m_receiverQueue;

};

#endif // MESSAGEHANDLER_H

