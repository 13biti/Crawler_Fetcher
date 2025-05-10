#ifndef URL_MANAGER_H
#define URL_MANAGER_H

#include "../include/ResolverHelper.h"
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/json.hpp>
#include <chrono>
#include <iostream>
#include <mongocxx/client.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
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
struct Bulk_Read {
  std::vector<Result_read> result_read;
  bool status = false;
};
class UrlManager {
public:
  UrlManager(mongocxx::pool &pool, // Takes reference to connection pool
             const std::string &database_name,
             const std::string &collection_name = "urls",
             const std::vector<std::string> allowedUrls = {})
      : pool_(pool), database_name_(database_name),
        collection_name_(collection_name), _allowedUrls(allowedUrls) {}

  bool sortingUrls(const std::string &url);
  bool sortingUrls(std::vector<std::string> urls);
  Bulk_Read getUrl(std::vector<std::string> domains);
  Result_read getUrl(std::string domain);

  std::set<std::string> getBaseMap() {
    if (collection_map.empty())
      initiateMap();
    return collection_map;
  }
  void newMapReaded() { map_updated = false; }
  ~UrlManager() {}
  bool map_initiated = false;
  bool map_updated = false;

private:
  std::vector<std::string> _allowedUrls;
  ResolverHelper resolverHelper;
  std::mutex _mutex;
  DomainResolver domainResolver;
  mongocxx::pool &pool_; // Reference to connection pool
  std::string database_name_;
  std::string collection_name_;
  bool connectionValidator(mongocxx::client &client) {
    std::lock_guard<std::mutex> lock(_mutex);
    try {
      // Get admin database and run ping command
      auto admin_db = client["admin"];
      auto ping_cmd = bsoncxx::builder::stream::document{}
                      << "ping" << 1 << bsoncxx::builder::stream::finalize;

      auto result = admin_db.run_command(ping_cmd.view());
      return true;
    } catch (const std::exception &e) {
      std::cerr << "Connection validation failed: " << e.what() << std::endl;
      return false;
    }
  }

  bool allowed(const std::string &url);
  mongocxx::collection getCollection() {
    auto client = pool_.acquire();
    return (*client)[database_name_][collection_name_];
  }

  mongocxx::database getDatabase() {
    auto client = pool_.acquire();
    return (*client)[database_name_];
  }

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
