#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cerrno>
#include <regex>
#include <map>
#include <unistd.h>
#include <cstdlib>
#include "FileOperations.h"
#include "UrlManager.h"
#include "Politeness.h"
#include "QueueManager.h"

using namespace std;
Politeness politeness(2*60);
FileOperations fileOperator;
UrlManager urlManager(fileOperator);
QueueManager queueManager ("localhost" ,   5672 );

int main () {
    queueManager.createQueue("test");
    fileOperator.updateListForFirstTime();
    urlManager.sortingUrls("/home/kk_gorbee/Documents/project/Fetcher/mainProgram/test.txt");
    for(const auto& element : fileOperator.filesList) {
        if (politeness.politenessMap.find(element) == politeness.politenessMap.end())
            politeness.updatePoliteList(element, 180);
    }

    while (1) {
        usleep(1000000);
        politeness.Timer();
        for (auto itr =politeness.politenessMap.begin() ; itr !=politeness.politenessMap.end() ; ++itr) {
            if (itr->second == 0) {
                string Url = urlManager.getUrl(itr->first);
                if (Url == "") {
                    politeness.emptyDomainDeclaration(itr->first, -1);
                    continue;
                }
                queueManager.sendMessage("test" , Url);
                politeness.emptyDomainDeclaration(itr->first, 0);
            }
        }
    }

    //  list1.updatePoliteList();
//  SortingUrls("./test.txt");
//  std::cout << "done sorting "<<endl;
//  while (1){
//    usleep(1000000);
//    cout << "done ss "<<endl;
//    list1.Timer();
//    cout<< "out of timer  "<<endl;
}
//queue example : 
/*

int main() {
    const char* hostname = "localhost";
    const int port = 5672;
    const std::string queue_name = "test";

    AMQPConnection connection(hostname, port);

    // Send message
    std::string message_to_send = "Hello, World!";
    bool sent_successfully = connection.sendMessage(queue_name, message_to_send);
    if (sent_successfully) {
        std::cout << "Message sent successfully!" << std::endl;
    } else {
        std::cerr << "Error sending message" << std::endl;
        return EXIT_FAILURE;
    }

    // Receive message
    std::string received_message = connection.receiveMessage(queue_name);
    if (!received_message.empty()) {
        std::cout << "Received message: " << received_message << std::endl;
    } else {
        std::cerr << "Error receiving message" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
*/
