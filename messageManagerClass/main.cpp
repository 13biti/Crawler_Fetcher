#include<iostream>
#include "messageManagerClass.h"
int main() {
    const char* host = "localhost";
    int port = 5672;
    const char* username = "guest";
    const char* password = "guest";
    const char* queueName = "test";
    MessageHandler manager (host, port, username, password ,queueName, queueName);
    manager.sendMessage("Hello World!");
    manager.sendMessage("Hello World!1");
    manager.sendMessage("Hello World!2");
    manager.receiveMessage2();
    int i = 0;
    while (1){
      i++;
      i--;
    }


    return 0;
}


