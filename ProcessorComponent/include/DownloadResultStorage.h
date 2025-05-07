#ifndef DOWNLOAD_RESULT_STORAGE_H
#define DOWNLOAD_RESULT_STORAGE_H

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

class DownloadResultStorage {
public:
  struct result {
    std::string url;
    std::string base_url;
    std::string html_content;
    std::string timestamp;
    bool status = false;
  };
  // Constructor with default values
  DownloadResultStorage(
      const std::string &mongo_uri = "mongodb://localhost:27017/",
      const std::string &database_name = "testDb",
      const std::string &client_name = "admin");

  // Store a DownloadResult in MongoDB
  bool storeDownloadResult(const result &result);

  // Check connection status
  bool isConnected() const { return is_connected_; }

private:
  // Connection helper
  void connectToMongoDB(const std::string &mongo_uri,
                        const std::string &database_name,
                        const std::string &client_name);

  // MongoDB members
  mongocxx::instance instance_; // The MongoDB instance (must remain alive)
  mongocxx::client client_;     // MongoDB client
  mongocxx::database database_; // Database reference
  bool is_connected_;           // Connection status
};

#endif // DOWNLOAD_RESULT_STORAGE_H
