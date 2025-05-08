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
    std::string html_content_base64;
    std::string html_content;
    std::string timestamp;
    bool status = false;
  };
  DownloadResultStorage(
      mongocxx::client client, const std::string &database_name,
      const std::string &collection_name = "DownloadedContent")
      : client_(std::move(client)), database_(client_[database_name]),
        collection_(database_[collection_name]) {}
  bool storeDownloadResult(const result &result);

  bool isConnected() const { return is_connected_; }

private:
  void connectToMongoDB(const std::string &mongo_uri,
                        const std::string &database_name,
                        const std::string &client_name);

  mongocxx::client client_;
  mongocxx::database database_;
  mongocxx::collection collection_;
  bool is_connected_;
};

#endif // DOWNLOAD_RESULT_STORAGE_H
