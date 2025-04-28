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
#include <regex>
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
  UrlManager(const std::string &mongo_uri = "mongodb://localhost:27017/",
             const std::string &database_name = "testDb",
             const std::string &client_name = "admin")
      : instance_{}, client_{mongocxx::uri{mongo_uri}}, is_connected_{false} {
    connectToMongoDB(mongo_uri, database_name, client_name);
  }
  bool sortingUrls(std::vector<std::string> urls);
  bool sortingUrls(const std::string &url);
  std::vector<Result_read> getUrl(std::vector<std::string> domains);
  Result_read getUrl(std::string domain);
  std::unordered_map<std::string, std::string> getCollectionNames();
  ~UrlManager() {}
  bool map_initiated = false;
  bool map_updated = false;

private:
  // from up to down , instance of mongo , then a client , ...
  mongocxx::instance instance_;
  mongocxx::client client_;
  mongocxx::database database_;
  mongocxx::collection collection_;
  bool is_connected_ = false;
  void retryConnection(int interval_seconds);
  void updateMap(std::unordered_map<std::string, std::string> &target,
                 std::string key, std::string value);
  void connectToMongoDB(const std::string &mongo_uri,
                        const std::string &database_name,
                        const std::string &client_name) {
    try {
      // Access the "admin" database and run the ping command
      auto admin = client_[client_name];
      admin.run_command(bsoncxx::from_json(R"({ "ping": 1 })"));

      // Access the specified database
      database_ = client_[database_name];

      // If no exception is thrown, the connection is successful
      is_connected_ = true;
      std::cout << "Successfully connected to MongoDB!" << std::endl;
    } catch (const mongocxx::exception &e) {
      std::cerr << "An exception occurred: " << e.what() << std::endl;
      is_connected_ = false;
    }
  }
};

#endif // URL_MANAGER_H
