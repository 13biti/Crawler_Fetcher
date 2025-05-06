#include "../include/Politeness.h"
#include "../include/QueueManager.h"
#include "../include/UrlManager.h"
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <future>
#include <iostream>
#include <set>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>
#define CONSUMER_QUEUE_NAME "Ready-to-use-links"
#define NEW_LINKS_QUEUE_BASE_URL "http://127.0.0.1:5000"
#define NEW_LINK_PEER_SORT 10

using namespace std;
const std::string WRITE_USERNAME = "u2";
const std::string READ_USERNAME = "u1";
const std::string PASSWORD = "123";
const std::string API_SEND = "write";
const std::string API_RECEIVE = "read";
const std::string QUEUE_NAME = "test_queue";
auto envReader = [](const char *env_name,
                    const char *default_value) -> const char * {
  std::cout << env_name << std::endl;
  const char *env_value = std::getenv(env_name);
  return env_value ? env_value : default_value;
};
const std::string NEW_LINKS_QUEUE_NAME =
    envReader("NEW_LINKS_QUEUE_NAME", "newlinks");
const std::string NEW_LINK_QUEUE_READ_USERNAME =
    envReader("NEW_LINK_QUEUE_READ_USERNAME", "u");
const std::string NEW_LINKS_QUEUE_PASSWORD =
    envReader("NEW_LINKS_QUEUE_PASSWORD", "123");
const std::string API_LOGIN = envReader("API_LOGIN", "login");
const std::string MONGO_URLS_URI = envReader("MONGO_URLS_URI", "");
const std::string MONGO_URLS_DB = envReader("MONGO_URLS_DB", "");
const std::string MONGO_URLS_CLIENT = envReader("MONGO_URLS_CLIENT", "");

UrlManager *urlManager;
Politeness *politeness;
void readNewLinks() {
  // token.empty() should checked here !
  std::future<bool> futureResult;
  bool result;

  QueueManager *newLinksQueue;
  newLinksQueue = new QueueManager(NEW_LINKS_QUEUE_BASE_URL);

  newLinksQueue->getToken(READ_USERNAME, NEW_LINKS_QUEUE_PASSWORD, API_LOGIN);
  // std::cout << newLinksQueue->returnToken() << std::endl;
  std::vector<std::string> newLinks;

  // self log here , write lambeda , get like 10 link , async the liink manager
  // and then get link again , dont waist any time on writing dbs !!
  auto getLink = [&newLinks, newLinksQueue]() -> void {
    newLinks.clear();
    for (int i = 0; i < NEW_LINK_PEER_SORT; i++) {
      std::string newLink = newLinksQueue->receiveMessage(NEW_LINKS_QUEUE_NAME);
      if (!newLink.empty())
        newLinks.push_back(newLink);
      else
        break;
    }
  };
  // its hard to wrok in async !
  futureResult = std::async(std::launch::async, []() { return true; });
  int i = 10;
  while (i > 0) {
    try {
      getLink();
      if (futureResult.valid()) {
        result = futureResult
                     .get(); // Get the result of the previous async operation
      }

      if (newLinks.empty()) {
        std::cout << "iam empty" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        continue;
      }
      // static_cast<bool
      // (UrlManager::*)(std::vector<std::string>)>(&UrlManager::sortingUrls)
      // this line is trying to specefiy which one of sortingUrls is to use ,
      // case i have 2 of them , base on this , i tell it the one that get
      // vector of string and return bool
      futureResult = std::async(
          std::launch::async,
          static_cast<bool (UrlManager::*)(std::vector<std::string>)>(
              &UrlManager::sortingUrls),
          urlManager, newLinks);

    } catch (const std::future_error &ex) {
      std::cerr << "Future error: " << ex.what() << std::endl;
      futureResult = std::async(std::launch::async, []() { return true; });
    }
    i--;
  }
}
int main() {
  std::set<std::string> _urlMap;
  urlManager = new UrlManager(MONGO_URLS_URI, MONGO_URLS_DB, MONGO_URLS_CLIENT);
  // readNewLinks();
  politeness = new Politeness();
  auto updateCollectionMap = [&]() -> void {
    _urlMap = urlManager->getBaseMap();
    printf("iam here ");
  };
  updateCollectionMap();
  for (const auto &key : _urlMap) {

    std::cout << key << std::endl;
  }
  politeness->addJobs(_urlMap);
  politeness->displayStrHeapSorted();
  Politeness::StrJob newjob = politeness->getReadyJobStr();
  std::cout << newjob.nodeName << std::endl;
  sleep(5);
  newjob = politeness->getReadyJobStr();
  std::cout << newjob.nodeName << std::endl;
  politeness->displayStrHeapSorted();
  sleep(5);
  newjob = politeness->getReadyJobStr();
  std::cout << newjob.nodeName << std::endl;
  politeness->displayStrHeapSorted();
  sleep(5);
  newjob = politeness->getReadyJobStr();
  std::cout << newjob.nodeName << std::endl;
  politeness->displayStrHeapSorted();
  sleep(5);
  newjob = politeness->getReadyJobStr();
  std::cout << newjob.nodeName << std::endl;
  politeness->displayStrHeapSorted();
  return 0;
}

/*
classass Politeness {
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
*/

/*
int main() {

  Politeness politeness(10);

  // queueManager.createQueue("test");
  fileOperator.updateListForFirstTime();
  urlManager.sortingUrls(
      "/home/kk_gorbee/Documents/project/Fetcher/mainProgram/test.txt");
  politeness.updateLinkMap();
  std::string result;
  while (1) {
    usleep(100000);
    politeness.Timer();
    result = queueManager.receiveMessage();
    std::cout << result << "   ----" << std::endl;
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
// queue example :
/*

int main() {
    const char* hostname = "localhost";
    const int port = 5672;
    const std::string queue_name = "test";

    AMQPConnection connection(hostname, port);

    // Send message
    std::string message_to_send = "Hello, World!";
    bool sent_successfully = connection.sendMessage(queue_name,
message_to_send); if (sent_successfully) { std::cout << "Message sent
successfully!" << std::endl; } else { std::cerr << "Error sending message" <<
std::endl; return EXIT_FAILURE;
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
