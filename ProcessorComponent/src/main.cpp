#include "../../Common/Configs/Config.h"
#include "../../Common/QueueManager/QueueManager.h"
#include "../../DownloaderComponent/include/CurlDownloder.h"
#include "../include/DownloadResultStorage.h"
#include "../include/LinkExtractor.h"
#include "../include/Mongo.h"
#include "../include/Politeness.h"
#include "../include/UrlManager.h"
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <future>
#include <iostream>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#include <mutex>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define NEW_LINK_PEER_SORT 10

using json = nlohmann::json;
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
void threadWrite(UrlManager *urlManager, Politeness *politeness,
                 int linkPerTrun) {
  std::cout << "threadWrite is initiated " << std::endl;
  try {
    Politeness::JotDto newjob;
    std::set<std::string> _urlMap;
    QueueManager *newLinksQueue;
    newLinksQueue = new QueueManager(Config::queueBaseUrl);
    newLinksQueue->getToken(Config::processorWriteUsername,
                            Config::queuePassword, Config::apiLogin);
    auto token = newLinksQueue->returnToken();

    auto sendLink = [newLinksQueue, token](std::string downloadbleUrl,
                                           std::string base_url) -> void {
      json jsonPayload;
      jsonPayload["Url"] = downloadbleUrl;
      jsonPayload["base_url"] = base_url;
      std::string jsonString = jsonPayload.dump();
      std::cout << "[writer] i am sending this url and jobid " << downloadbleUrl
                << " \t " + base_url << std::endl;
      newLinksQueue->sendMessage(Config::downloadLinksQueueName, jsonString,
                                 token, Config::apiSend);
    };
    // you mayfor  ask why i have this conditions ? for not reading database
    // every time someting happen there !!
    auto updateCollectionMap = [&]() -> bool {
      if (!urlManager->map_initiated || urlManager->map_updated) {
        std::cout << "[updateCollectionMap] why i am here "
                  << !urlManager->map_initiated << urlManager->map_updated
                  << std::endl;
        _urlMap = urlManager->getBaseMap();
        if (!_urlMap.empty()) {
          std::cout << "[updateCollectionMap] i want to update polite here is "
                       "before :  "
                    << std::endl;
          politeness->displayStrHeap();
          urlManager->newMapReaded();
          politeness->addJobs(_urlMap);
          std::cout << "[updateCollectionMap] i want to update polite here is "
                       "after :  "
                    << std::endl;
          politeness->displayStrHeap();
        } else
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
        std::cout << "new job statuss " << newjob.base_url << newjob.id
                  << newjob.status << std::endl;
        if (newjob.status)
          break;
        sleep(1);
      }

      auto downloadbleUrls = urlManager->getUrl(
          std::vector<std::string>(linkPerTrun, newjob.base_url));
      std::cout << "[writer][sender] enless while , here is linkPerTrun"
                << linkPerTrun << "and here is downloadbleUrls from getter : "
                << downloadbleUrls.status << " "
                << (downloadbleUrls.status
                        ? downloadbleUrls.result_read[0].message
                        : "it was empty ")
                << "here is what i sende "
                << std::vector<std::string>(linkPerTrun, newjob.base_url)[0]
                << std::endl;
      if (downloadbleUrls.status) {
        std::cout << "[writer][sender] enless while try to suspend this job "
                  << newjob.id << std::endl;
        politeness->SuspendJob(newjob.id);
        for (const auto it : downloadbleUrls.result_read) {
          if (it.status) {
            sendLink(it.message, newjob.base_url);
          }
        }
      } else {
        std::cout << "[writer][sender] enless while , here is linkPerTrun"
                  << linkPerTrun << "here is the base_url" << newjob.base_url
                  << std::endl;
        politeness->NAckJob(newjob.id);
        counter++;
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "[writer] CRASH: " << e.what() << std::endl;
    std::abort(); // Force core dump
  }
  std::cout << "[writer] Thread exiting\n";
}
void threadDownloadResultHandler(DownloadResultStorage *storage,
                                 Politeness &politeness, int linkPerTrun) {
  std::cout << "threadDownloadResltHandler is initiated\n";
  std::unordered_map<int, int> DownloadedPerWindow;
  // everything in try becase of the thread
  try {
    LinkExtractor linkExtractor; // No need for dynamic allocation
    QueueManager newLinksQueue(Config::queueBaseUrl);

    auto updatehelper = [](bool &var, bool stat) -> int {
      var = stat;
      // this 0 here , toke me about 1 hour to find and solve !!
      //  return 0;
      return 1;
    };
    auto update_DPW_map = [&DownloadedPerWindow, linkPerTrun,
                           updatehelper](int jobId) -> bool {
      bool ackTheJob = false;
      auto it = DownloadedPerWindow.find(jobId);
      if (it == DownloadedPerWindow.end())
        DownloadedPerWindow.insert({jobId, 1});
      else
        it->second = (it->second >= (linkPerTrun - 1))
                         ? updatehelper(ackTheJob, true)
                         : it->second + 1;
      for (const auto iu : DownloadedPerWindow) {
        std::cout << "[getResult] updating window like this :" << jobId
                  << "and " << iu.first << " here is secondg " << iu.second
                  << std::endl;
      }
      return ackTheJob;
    };
    auto readToken = newLinksQueue.getToken(
        Config::processorReadUsername, Config::queuePassword, Config::apiLogin);

    auto writeToken =
        newLinksQueue.getToken(Config::processorWriteUsername,
                               Config::queuePassword, Config::apiLogin);

    auto getResult = [&newLinksQueue, &readToken, &politeness, update_DPW_map,
                      &DownloadedPerWindow]() -> DownloadResultStorage::result {
      auto msg = newLinksQueue.receiveMessage(Config::downloadedQueueName,
                                              readToken, Config::apiReceive);
      if (!msg.status) {
        return DownloadResultStorage::result();
      }
      try {
        auto j = json::parse(msg.message);
        DownloadResult res = DownloadResult::from_json(j);
        std::cout << "[getResult] of resulthandler , here is doresult  : "
                  << res.url << res.error_message << "hereis jobid "
                  << res.base_url << std::endl;
        int jobId = politeness.getIdByNodeName(res.base_url);
        bool ackTheJob = update_DPW_map(jobId);
        auto it = DownloadedPerWindow.find(jobId);
        std::cout << "[getResult][dpw] update dwp_map : " << jobId << " "
                  << it->first << "and here is value " << it->second
                  << "ack is " << ackTheJob << std::endl;
        if (ackTheJob)
          politeness.AckJob(jobId);
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
              res.url,      res.base_url,  res.html_content_base64,
              decoded_html, res.timestamp, true};
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
          auto rawLinks = linkExtractor.ExtractRedirectLinks(res.html_content,
                                                             res.base_url);

          // Remove duplicates more efficiently
          std::unordered_set<std::string> uniqueLinks(rawLinks.begin(),
                                                      rawLinks.end());
          sendRawLinks(
              std::vector<std::string>(uniqueLinks.begin(), uniqueLinks.end()));
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
    std::cerr << "[resulthandler] CRASH: " << e.what() << "\n";
    std::abort();
  }

  std::cout << "[resulthandler] Thread exiting\n";
}
// iam use threading for simplisity , case if i fork them , i need to have
// shared memory and stuff to share the urlManager between this two method
// several problem here : first , i remove everything in queue after i read it
// , which wrong , i sholud acknowledgeMessage insted second : is token
// condition , as you know thy have expireion time,but i never recreate them ,
// thred : i am using same username and pass for them all , but what if thy
// start to change each other token and stuff ?
int main(int argc, char *argv[]) {
  std::vector<std::string> allowedUrls;
  int linkPerTrun = 1;

  for (int i = 1; i < argc; ++i) {
    std::string arg(argv[i]);
    if (arg.rfind("--allowed-url=", 0) == 0) {
      std::string url = arg.substr(strlen("--allowed-url="));
      allowedUrls.push_back(url);
    }
    if (arg.rfind("--LPT=", 0) == 0) {
      linkPerTrun = std::stoi(arg.substr(strlen("--LPT=")));
    }
  }

  for (const auto &url : allowedUrls) {
    std::cout << "Allowed URL: " << url << std::endl;
  }
  std::cout << "Allowed ltp : " << linkPerTrun << std::endl;
  // Print configuration values
  std::cout << Config::mongoUrlsDb << Config::mongoUrlsClient
            << Config::mongoHandlerClient << Config::mongoUrlsUri
            << Config::downloadedQueueName << Config::queueBaseUrl << std::endl;

  // MongoDB instance (must be created before any other MongoDB operations)
  auto &mongoInstance = MongoDB::getInstance();
  (void)mongoInstance;

  // Create connection pool
  mongocxx::pool pool{mongocxx::uri{Config::mongoUrlsUri}};

  // Function to validate a client connection
  auto validate_client = [](mongocxx::client &client) {
    try {
      auto admin_db = client["admin"];
      auto ping_cmd = bsoncxx::builder::stream::document{}
                      << "ping" << 1 << bsoncxx::builder::stream::finalize;
      admin_db.run_command(ping_cmd.view());
      std::cout << "MongoDB connection successful." << std::endl;
      return true;
    } catch (const mongocxx::exception &e) {
      std::cerr << "MongoDB connection failed: " << e.what() << std::endl;
      return false;
    }
  };

  // Test the pool by getting and validating a client
  {
    auto client = pool.acquire();
    if (!validate_client(*client)) {
      std::cerr << "Failed to initialize MongoDB connection pool" << std::endl;
      return EXIT_FAILURE;
    }
  }

  // Create managers with the connection pool
  UrlManager urlManager(pool, Config::mongoUrlsDb, "urls", allowedUrls);
  DownloadResultStorage storage(pool, Config::mongoUrlsDb, "DownloadedContent");
  Politeness politeness;

  // Start threads
  std::thread reader(threadRead, &urlManager);
  std::thread writer(threadWrite, &urlManager, &politeness, linkPerTrun);
  std::thread handler([&storage, &politeness, linkPerTrun] {
    threadDownloadResultHandler(&storage, politeness, linkPerTrun);
  });

  // Wait for threads to complete
  writer.join();
  reader.join();
  handler.join();

  return EXIT_SUCCESS;
}
