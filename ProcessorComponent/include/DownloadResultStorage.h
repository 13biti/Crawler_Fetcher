#ifndef DOWNLOAD_RESULT_STORAGE_H
#define DOWNLOAD_RESULT_STORAGE_H

#include "ResolverHelper.h"
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/uri.hpp>
#include <mutex>

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
      mongocxx::pool &pool, const std::string &database_name,
      const std::string &collection_name = "DownloadedContent")
      : pool_(pool), database_name_(database_name),
        collection_name_(collection_name) {}

  bool storeDownloadResult(const result &result);

  bool isConnected() const {
    std::lock_guard<std::mutex> lock(connection_mutex_);
    return is_connected_;
  }

private:
  mongocxx::pool &pool_; // Reference to connection pool
  std::string database_name_;
  std::string collection_name_;
  mutable std::mutex connection_mutex_;
  bool is_connected_ = true; // Pool manages connections, so we assume connected

  // Helper method to get a collection handle
  mongocxx::collection getCollection() {
    auto client = pool_.acquire();
    return (*client)[database_name_][collection_name_];
  }

  // Helper method to get a database handle
  mongocxx::database getDatabase() {
    auto client = pool_.acquire();
    return (*client)[database_name_];
  }

  // Validate connection using a client from the pool
  bool validateConnection() {
    try {
      auto client = pool_.acquire();
      auto admin_db = (*client)["admin"];
      auto ping_cmd = bsoncxx::builder::stream::document{}
                      << "ping" << 1 << bsoncxx::builder::stream::finalize;
      admin_db.run_command(ping_cmd.view());
      return true;
    } catch (const std::exception &e) {
      std::lock_guard<std::mutex> lock(connection_mutex_);
      is_connected_ = false;
      std::cerr << "Connection validation failed: " << e.what() << std::endl;
      return false;
    }
  }
};

#endif // DOWNLOAD_RESULT_STORAGE_H
