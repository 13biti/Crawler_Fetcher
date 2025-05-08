#ifndef URL_MANAGER_H
#define URL_MANAGER_H
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/json.hpp>
#include <chrono>
#include <iostream>
#include <mongocxx/client.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mutex>
#include <regex>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
struct Result_read {
  bool status;
  std::string message = "";
};
class UrlManager {
public:
  UrlManager(mongocxx::client client, // Takes ownership
             const std::string &database_name,
             const std::string &collection_name = "urls")
      : client_(std::move(client)), database_(client_[database_name]),
        collection_(database_[collection_name]) {}
  bool sortingUrls(const std::string &url);
  bool sortingUrls(std::vector<std::string> urls);
  std::vector<Result_read> getUrl(std::vector<std::string> domains);
  Result_read getUrl(std::string domain);
  std::set<std::string> getBaseMap() {
    if (collection_map.empty())
      initiateMap();
    return collection_map;
  }
  ~UrlManager() {}
  bool map_initiated = false;
  bool map_updated = false;

private:
  std::mutex _mutex;
  bool connectionValidator() {
    std::lock_guard<std::mutex> lock(_mutex);
    try {
      // Check client connection status (correct way)
      if (!client_) { // Removed the () - client_ is an object, not callable
        std::cerr << "MongoDB client is invalid!" << std::endl;
        return false;
      }

      // Get admin database and run ping command
      auto admin_db = client_["admin"];
      auto ping_cmd = bsoncxx::builder::stream::document{}
                      << "ping" << 1 << bsoncxx::builder::stream::finalize;

      // Correct way to run command (on database, not collection)
      auto result = admin_db.run_command(
          ping_cmd.view()); // This is correct for mongocxx v3.7.0+
      return true;
    } catch (const std::exception &e) {
      std::cerr << "Connection validation failed: " << e.what() << std::endl;
      return false;
    }
  }
  mongocxx::client client_;
  mongocxx::database database_;
  mongocxx::collection collection_;
  bool is_connected_ = false;
  std::set<std::string> collection_map;
  void retryConnection(int interval_seconds);
  void initiateMap() {
    printf("it was empty -- ");
    auto result = getBaseUrls();
    if (!result.empty() && map_initiated) {
      collection_map = result;
    } else {
      collection_map.clear();
    }
  }
  void updateMap(std::set<std::string> &target, std::string key);
  void updateMap(std::unordered_map<std::string, std::string> &target,
                 std::string key, std::string value);

  std::unordered_map<std::string, std::string> getCollectionNames();
  std::set<std::string> getBaseUrls();
};

#endif // URL_MANAGER_H
