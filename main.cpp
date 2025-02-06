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
#include "/home/kk_gorbee/Documents/project/Fetcher/mainProgram/messageManagerClass/messageManagerClass.h"

using namespace std;

FileOperations fileOperator;
UrlManager urlManager(fileOperator);
MessageHandler queueManager("localhost" ,5672 , "guest" , "guest" , "downloaderA_result" , "test");

class Politeness {
private:
    int DefaultTimer;

public:
    Politeness(int fixTimer) : DefaultTimer(fixTimer) {}
    std::map<std::string, int> politenessMap;
    void updateLinkMap(){
        for(const auto& element : fileOperator.filesList) {
            std::cout<<element<<std::endl;
            if (politenessMap.find(element) == politenessMap.end()) {
                politenessMap[element] = DefaultTimer;
            }
            //updatePoliteList(element, 180);
        }
    }

    void updatePoliteList(std::string websiteName, int timeCounter) {
        politenessMap[websiteName] = timeCounter;
    }

    void Timer() {
        for (auto &pair : politenessMap) {
            std::cout<<"iam in timer"<<std::endl;
            if (pair.second > 0) {
                pair.second -= 1;
                std::cout<<pair.second<<std::endl;
                if (pair.second == 0) {
                    string Url = urlManager.getUrl(pair.first);
                    if (!Url.empty()) {
                        queueManager.sendMessage( Url);
                    }
                    emptyDomainDeclaration(pair.first, -1);
                }
            }
        }
    }

    void emptyDomainDeclaration(std::string key, int value) {
        int x = value < 0 ? -1 : DefaultTimer;
        politenessMap[key] = x;
    }
    void domainUnlocker(std::string result){  
      size_t loc = result.find("~");
      int resultNumber = stoi(result.substr(0 , loc));
      std::string RUrl = result.substr(loc+1);
      //std::cout<<resultNumber<<RUrl<<std::endl;
        string tmp = std::string();
        std::string target = tmp ;
         smatch domain;
        string UrlFilePath;
        regex pattern ("(?:https?://)([^/]+)");
        if (std::regex_search(target, domain, pattern))
            UrlFilePath = domain.str(1)+".csv";

      if (resultNumber == 0)
          emptyDomainDeclaration(UrlFilePath , 1);
    }
};

int main () {

    Politeness politeness(10);

    //queueManager.createQueue("test");
    fileOperator.updateListForFirstTime();
    urlManager.sortingUrls("/home/kk_gorbee/Documents/project/Fetcher/mainProgram/test.txt");
    politeness.updateLinkMap();
    std::string result ;
    while (1) {
        usleep(100000);
        politeness.Timer();
        result = queueManager.receiveMessage();
        std::cout<<result<<"   ----" << std::endl;
        if (result == "@")
            continue;
        politeness.domainUnlocker(result);
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
