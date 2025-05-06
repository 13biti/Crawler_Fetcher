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
#include <signal.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>
#define CONSUMER_QUEUE_NAME "Ready-to-use-links"
#define NEW_LINKS_QUEUE_BASE_URL "http://127.0.0.1:5000"
#define NEW_LINK_PEER_SORT 10

struct Config {
  static std::string getenvOrDefault(const char *name, const char *defaultVal) {
    const char *val = std::getenv(name);
    return val ? val : defaultVal;
  }
  static inline const std::string rawLinksQeueuName =
      getenvOrDefault("RAW_LINKS_QUEUE_NAME", "rawlinks");
  static inline const std::string downloadLinksQeueuName =
      getenvOrDefault("DOWNLOAD_LINKS_QUEUE_NAME", "downloadlinks");
  static inline const std::string writeUsername =
      getenvOrDefault("WRITE_USERNAME", "u2");
  static inline const std::string readUsername =
      getenvOrDefault("READ_USERNAME", "u1");
  static inline const std::string queuePassword =
      getenvOrDefault("QUEUE_PASSWORD", "123");
  static inline const std::string apiLogin =
      getenvOrDefault("API_LOGIN", "login");
  static inline const std::string apiSend =
      getenvOrDefault("API_SEND", "/write");
  static inline const std::string apiReceive =
      getenvOrDefault("API_RECEIVE", "/read");
  static inline const std::string mongoUrlsUri =
      getenvOrDefault("MONGO_URLS_URI", "");
  static inline const std::string mongoUrlsDb =
      getenvOrDefault("MONGO_URLS_DB", "");
  static inline const std::string mongoUrlsClient =
      getenvOrDefault("MONGO_URLS_CLIENT", "");
};
UrlManager *urlManager;
Politeness *politeness;
void threadRead(UrlManager *urlManager) {
  // token.empty() should checked here !
  std::future<bool> futureResult;
  bool result;

  QueueManager *newLinksQueue;
  newLinksQueue = new QueueManager(NEW_LINKS_QUEUE_BASE_URL);

  newLinksQueue->getToken(Config::readUsername, Config::queuePassword,
                          Config::apiLogin);
  // std::cout << newLinksQueue->returnToken() << std::endl;
  std::vector<std::string> newLinks;

  // self log here , write lambeda , get like 10 link , async the liink manager
  // and then get link again , dont waist any time on writing dbs !!
  auto getLink = [&newLinks, newLinksQueue]() -> void {
    newLinks.clear();
    for (int i = 0; i < NEW_LINK_PEER_SORT; i++) {
      std::string newLink =
          newLinksQueue->receiveMessage(Config::rawLinksQeueuName);
      if (!newLink.empty())
        newLinks.push_back(newLink);
      else
        break;
    }
  };
  // its hard to wrok in async !
  futureResult = std::async(std::launch::async, []() { return true; });
  while (true) {
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
  }
}
void threadWrite(UrlManager *urlManager, Politeness *politeness) {
  Politeness::JotDto newjob;
  std::set<std::string> _urlMap;
  QueueManager *newLinksQueue;
  newLinksQueue = new QueueManager(NEW_LINKS_QUEUE_BASE_URL);
  newLinksQueue->getToken(Config::writeUsername, Config::queuePassword,
                          Config::apiLogin);
  auto token = newLinksQueue->returnToken();

  auto sendLink = [newLinksQueue, token](std::string downloadbleUrl) -> void {
    for (int i = 0; i < NEW_LINK_PEER_SORT; i++) {
      newLinksQueue->sendMessage(Config::downloadLinksQeueuName, downloadbleUrl,
                                 token, Config::apiSend);
    }
  };
  // you may ask why i have this conditions ? for not reading database every
  // time someting happen there !!
  auto updateCollectionMap = [&]() -> void {
    if (!urlManager->map_initiated || urlManager->map_updated) {
      _urlMap = urlManager->getBaseMap();
      politeness->addJobs(_urlMap);
    }
  };
  while (true) {
    // i may need to call this function in periods , like in loop :
    updateCollectionMap();
    while (true) {
      newjob = politeness->getReadyJobStr();
      if (newjob.status)
        break;
      sleep(1);
    }
    auto downloadbleUrl = urlManager->getUrl(newjob.base_url);
    if (downloadbleUrl.status)
      sendLink(downloadbleUrl.message);
  }
}

int main() {
  urlManager = new UrlManager(Config::mongoUrlsUri, Config::mongoUrlsDb,
                              Config::mongoUrlsClient);
  politeness = new Politeness();
  std::thread reader(threadRead, urlManager);
  std::thread writer(threadWrite, urlManager, politeness);
  writer.join();
  reader.join();
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
