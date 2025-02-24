#include "../include/UrlManager.h"
#include <bsoncxx/builder/basic/document.hpp>
#include <iostream>

#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>

#include <mongocxx/exception/exception.hpp>

#include <mongocxx/instance.hpp>

#include <mongocxx/uri.hpp>
#include <vector>
// Define fileOperator as a static member of UrlManager
void UrlManager::retryConnection(int interval_seconds = 10) {
  while (!is_connected_) {
    std::cout << "Retrying connection to MongoDB..." << std::endl;
    connectToMongoDB("mongodb://localhost:27017/", "testDb", "admin");
    std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
  }
}
bool UrlManager::sortingUrls(std::string url) {
  if (!is_connected_) {
    std::cerr << "connection is not established !" << std::endl;
    return false;
  }
  std::smatch result;
  std::regex pattern("(?:https?://)([^/]+)");
  if (std::regex_search(url, result, pattern)) {
    std::string target = result.str(1);
    auto collection = database_[target];
    // i like this use something like this :     auto result =
    // collection.insert_one(make_document(kvp("name", "p1")));
    // but it seems that error handeling in this method is better !!
    try {
      auto insert_result =
          collection.insert_one(bsoncxx::builder::stream::document{}
                                << "url" << url << "hasBeenRead" << false
                                << bsoncxx::builder::stream::finalize);

      if (insert_result) {
        std::cout << "URL inserted successfully: " << url << std::endl;
        return true;
      } else {
        std::cerr << "Failed to insert URL: " << url << std::endl;
        return false;
      }
    } catch (const mongocxx::exception &e) {
      std::cerr << "An exception occurred: " << e.what() << std::endl;
      return false;
    }
  } else {
    std::cerr << "Invalid URL format: " << url << std::endl;
    return false;
  }
}

bool UrlManager::sortingUrls(std::vector<std::string> urls) {
  if (!is_connected_) {
    std::cerr << "connection is not established !" << std::endl;
    return false;
  }
  bool all_success = true;
  for (const std::string &url : urls) {
    if (!sortingUrls(url)) {
      all_success = false;
    }
  }
  return all_success;
}
Result_read UrlManager::getUrl(std::string domain) {
  if (!is_connected_) {
    std::cerr << "Connection is not established!" << std::endl;
    return Result_read{false, "Connection is not established!"};
  }

  try {
    // agian i like this : auto results =
    // collection.find(make_document(kvp("<field name>", "<value>"))); but seems
    // ...
    auto collection = database_[domain];
    auto cursor = collection.find_one(bsoncxx::builder::stream::document{}
                                      << "hasBeenRead" << false
                                      << bsoncxx::builder::stream::finalize);

    if (cursor) {
      auto url = (*cursor)["url"].get_string().value;
      // change record(document) status to read
      collection.update_one(

          bsoncxx::builder::stream::document{}
              << "url" << url << bsoncxx::builder::stream::finalize,
          bsoncxx::builder::stream::document{}
              << "$set" << bsoncxx::builder::stream::open_document
              << "hasBeenRead" << true
              << bsoncxx::builder::stream::close_document
              << bsoncxx::builder::stream::finalize);
      return Result_read{true, std::string(url)};
    } else {
      return Result_read{false, "No URL found in collection: " + domain};
    }
  } catch (const mongocxx::exception &e) {
    return Result_read{false, "Error retrieving URL from collection: " +
                                  domain + " - " + e.what()};
  }
}
std::vector<Result_read> UrlManager::getUrl(std::vector<std::string> domains) {
  std::vector<Result_read> results;

  if (!is_connected_) {
    std::cerr << "Connection is not established!" << std::endl;
    results.push_back(Result_read{false, "Connection is not established!"});
    return results;
  }

  bool all_success = true;
  for (const std::string &domain : domains) {
    Result_read result = getUrl(domain);
    if (!result.status) {
      all_success = false;
    }
    results.push_back(result);
  }

  if (all_success) {
    std::cout << "All URLs retrieved successfully!" << std::endl;
  } else {
    std::cerr << "Some URLs could not be retrieved." << std::endl;
  }

  return results;
}
