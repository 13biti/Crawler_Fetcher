#include "../../Common/Configs/Config.h"
#include "../../Common/QueueManager/QueueManager.h"
#include "../../DownloaderComponent/include/CurlDownloder.h"
#include "../include/DownloadResultStorage.h"
#include "../include/LinkExtractor.h"
#include "../include/Mongo.h"
#include "../include/Politeness.h"
#include "../include/UrlManager.h"
#include <algorithm> // For std::find
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <future>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <unordered_set>
#include <vector>

#define NEW_LINK_PEER_SORT 10

UrlManager *urlManager;
Politeness *politeness;
std::mutex UrlManagerMutex;
void threadRead(UrlManager *urlManager) {
  // token.empty() should checked here !
  std::cout << "threadRead is initiated " << std::endl;
  try {
    std::future<bool> futureResult;
    bool result;

    QueueManager *newLinksQueue;
    newLinksQueue = new QueueManager(Config::queueBaseUrl);

    auto token = newLinksQueue->getToken(
        Config::processorReadUsername, Config::queuePassword, Config::apiLogin);
    // std::cout << newLinksQueue->returnToken() << std::endl;
    std::vector<std::string> newLinks;

    // self log here , write lambeda , get like 10 link , async the liink
    // manager and then get link again , dont waist any time on writing dbs !!
    auto getLink = [&newLinks, newLinksQueue, &token]() -> void {
      newLinks.clear();
      for (int i = 0; i < NEW_LINK_PEER_SORT; i++) {
        auto newLink = newLinksQueue->receiveMessage(Config::rawLinksQueueName,
                                                     token, Config::apiReceive);
        if (newLink.status)
          newLinks.push_back(newLink.message);
        else
          break;
      }
    };
    // its hard to wrok in async !
    futureResult = std::async(std::launch::async, []() { return true; });
    while (true) {
      try {
        getLink();

        if (newLinks.empty()) {
          std::this_thread::sleep_for(std::chrono::milliseconds(5000));
          continue;
        }
        // Synchronously call sortingUrls
        bool result = urlManager->sortingUrls(newLinks);
        // Optional: log result if needed
        // Sleep briefly to avoid hammering system
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

      } catch (const std::exception &ex) {
        std::cerr << "[reader] Exception: " << ex.what() << std::endl;
        std::this_thread::sleep_for(
            std::chrono::milliseconds(1000)); // brief backoff on error
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "[reader] CRASH: " << e.what() << std::endl;
    std::abort(); // Force core dump
  }
  std::cout << "[reader] Thread exiting\n";
}
void threadWrite(UrlManager *urlManager, Politeness *politeness) {
  std::cout << "threadWrite is initiated " << std::endl;
  try {
    Politeness::JotDto newjob;
    std::set<std::string> _urlMap;
    QueueManager *newLinksQueue;
    newLinksQueue = new QueueManager(Config::queueBaseUrl);
    newLinksQueue->getToken(Config::processorWriteUsername,
                            Config::queuePassword, Config::apiLogin);
    auto token = newLinksQueue->returnToken();

    auto sendLink = [newLinksQueue, token](std::string downloadbleUrl) -> void {
      newLinksQueue->sendMessage(Config::downloadLinksQueueName, downloadbleUrl,
                                 token, Config::apiSend);
    };
    // you mayfor  ask why i have this conditions ? for not reading database
    // every time someting happen there !!
    auto updateCollectionMap = [&]() -> bool {
      if (!urlManager->map_initiated || urlManager->map_updated) {
        std::cout << "-------------------------------if" << std::endl;
        _urlMap = urlManager->getBaseMap();
        if (!_urlMap.empty())
          politeness->addJobs(_urlMap);
        else
          return false;

        return true;
      }
      return false;
    };
    // this is for the very first time , i should have something to work with .
    // so first , it will initiate base map , if politeness have something to
    // work one , then it will update it and continue if not , then i will try
    // to rich it untill have something to work , so this while will break after
    // first true , first thing that politeness can work with it ;
    while (!updateCollectionMap()) {
      std::cout << "[wait] No URLs to process yet. Retrying in 5s...\n";
      std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    int counter = 0;
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
      if (downloadbleUrl.status) {
        politeness->AckJob(newjob.id);
        sendLink(downloadbleUrl.message);
      } else {
        politeness->NAckJob(newjob.id);
        counter++;
      }
      if (counter > 10) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(10000)); // brief backoff on error
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "[reader] CRASH: " << e.what() << std::endl;
    std::abort(); // Force core dump
  }
  std::cout << "[reader] Thread exiting\n";
}
void threadDownloadResltHandler(DownloadResultStorage *storage) {
  std::cout << "threadDownloadResltHandler is initiated\n";

  try {
    LinkExtractor linkExtractor; // No need for dynamic allocation
    QueueManager newLinksQueue(Config::queueBaseUrl);

    auto readToken = newLinksQueue.getToken(
        Config::processorReadUsername, Config::queuePassword, Config::apiLogin);

    auto writeToken =
        newLinksQueue.getToken(Config::processorWriteUsername,
                               Config::queuePassword, Config::apiLogin);

    auto getResult = [&newLinksQueue,
                      &readToken]() -> DownloadResultStorage::result {
      auto msg = newLinksQueue.receiveMessage(Config::downloadedQueueName,
                                              readToken, Config::apiReceive);
      if (!msg.status) {
        return DownloadResultStorage::result();
      }
      try {
        auto j = json::parse(msg.message);
        DownloadResult res = DownloadResult::from_json(j);
        if (res.http_code == 200) {
          std::string decoded_html;
          try {
            decoded_html = res.get_decoded_html();
          } catch (const std::exception &e) {
            std::cerr << "Base64 decode error for URL " << res.url << ": "
                      << e.what() << "\n";
            decoded_html = "";
          }

          return DownloadResultStorage::result{
              res.url,       "",  res.html_content_base64, decoded_html,
              res.timestamp, true};
        }
      } catch (const json::exception &e) {
        std::cerr << "JSON parsing error: " << e.what()
                  << "\nMessage: " << msg.message << "\n";
      }

      return DownloadResultStorage::result();
    };

    auto sendRawLinks = [&newLinksQueue,
                         &writeToken](const std::vector<std::string> &rawUrls) {
      for (const auto &url : rawUrls) {
        bool stat = newLinksQueue.sendMessage(Config::rawLinksQueueName, url,
                                              writeToken, Config::apiSend);
        if (!stat) {
          std::cerr << "Failed to send: " << url << std::endl;
        } else {
          std::cout << "Sent: " << url << std::endl;
        }
      }
    };

    int errorCounter = 0;
    const int maxErrorsBeforeBackoff = 10;
    const std::chrono::milliseconds shortSleep(1000);
    const std::chrono::milliseconds longSleep(10000);

    while (true) {
      auto res = getResult();

      if (res.status) {
        errorCounter = 0; // Reset error counter on success

        try {
          auto baseurl = linkExtractor.GetBaseUrl(res.url);
          auto rawLinks =
              linkExtractor.ExtractRedirectLinks(res.html_content, baseurl);

          // Remove duplicates more efficiently
          std::unordered_set<std::string> uniqueLinks(rawLinks.begin(),
                                                      rawLinks.end());
          sendRawLinks(
              std::vector<std::string>(uniqueLinks.begin(), uniqueLinks.end()));
          res.base_url = baseurl;
          storage->storeDownloadResult(res);
        } catch (const std::exception &e) {
          std::cerr << "Error processing URL " << res.url << ": " << e.what()
                    << "\n";
        }
      } else {
        errorCounter++;
        auto sleepDuration =
            errorCounter > maxErrorsBeforeBackoff ? longSleep : shortSleep;
        std::this_thread::sleep_for(sleepDuration);
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "[reader] CRASH: " << e.what() << "\n";
    std::abort();
  }

  std::cout << "[reader] Thread exiting\n";
}
// iam use threading for simplisity , case if i fork them , i need to have
// shared memory and stuff to share the urlManager between this two method
// several problem here : first , i remove everything in queue after i read it ,
// which wrong , i sholud acknowledgeMessage insted second : is token condition
// , as you know thy have expireion time,but i never recreate them , thred : i
// am using same username and pass for them all , but what if thy start to
// change each other token and stuff ?
int main() {
  std::cout << Config::mongoUrlsDb << Config::mongoUrlsClient
            << Config::mongoHandlerClient << Config::mongoUrlsUri
            << Config::downloadedQueueName << Config::queueBaseUrl << std::endl;
  auto &mongoInstance = MongoDB::getInstance(); // Now correctly static
  (void)mongoInstance;
  std::vector<mongocxx::client> clients;
  auto validate_client = [](mongocxx::client &client) {
    try {
      auto admin_db = client["admin"];
      auto ping_cmd = bsoncxx::builder::stream::document{}
                      << "ping" << 1 << bsoncxx::builder::stream::finalize;
      admin_db.run_command(ping_cmd.view()); // Throws if connection fails
      std::cout << "MongoDB connection successful." << std::endl;
      return true;
    } catch (const mongocxx::exception &e) {
      std::cerr << "MongoDB connection failed: " << e.what() << std::endl;
      return false;
    }
  };
  for (int i = 0; i < 3; ++i) {
    clients.emplace_back(mongocxx::uri{Config::mongoUrlsUri});
    if (!validate_client(clients.back())) {
      std::cerr << "Failed to initialize MongoDB client " << i << std::endl;
      return EXIT_FAILURE;
    }
  }
  UrlManager urlManager(std::move(clients[0]), Config::mongoUrlsDb, "urls");
  DownloadResultStorage storage(std::move(clients[2]), Config::mongoUrlsDb,
                                "DownloadedContent");
  Politeness politeness;

  std::thread reader(threadRead, &urlManager);
  std::thread writer(threadWrite, &urlManager, &politeness);
  std::thread handler([&storage] { threadDownloadResltHandler(&storage); });
  writer.join();
  reader.join();
  handler.join();
  return EXIT_SUCCESS;
}
