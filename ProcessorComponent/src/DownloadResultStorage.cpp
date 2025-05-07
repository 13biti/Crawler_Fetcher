#include "../include/DownloadResultStorage.h"
#include <iostream>

using namespace bsoncxx::builder::stream;

DownloadResultStorage::DownloadResultStorage(const std::string &mongo_uri,
                                             const std::string &database_name,
                                             const std::string &client_name)
    : instance_{}, client_{mongocxx::uri{mongo_uri}}, is_connected_{false} {
  connectToMongoDB(mongo_uri, database_name, client_name);
}

void DownloadResultStorage::connectToMongoDB(const std::string &mongo_uri,
                                             const std::string &database_name,
                                             const std::string &client_name) {
  try {
    // Ping the database to check connection
    auto admin_db = client_["admin"];
    auto ping_cmd = document{} << "ping" << 1 << finalize;
    admin_db.run_command(ping_cmd.view());

    // If we got here, connection is good
    database_ = client_[database_name];
    is_connected_ = true;
    std::cout << "Successfully connected to MongoDB: " << mongo_uri
              << std::endl;
  } catch (const mongocxx::exception &e) {
    std::cerr << "MongoDB connection error: " << e.what() << std::endl;
    is_connected_ = false;
  }
}

bool DownloadResultStorage::storeDownloadResult(const result &result) {
  if (!is_connected_) {
    std::cerr << "Connection is not established!" << std::endl;
    return false;
  }

  try {
    auto collection = database_["download_results"];

    // Create a document with all DownloadResult fields
    auto doc = document{} << "url" << result.url << "timestamp"
                          << result.timestamp << "content_length"
                          << static_cast<int64_t>(result.html_content.length())
                          << "html_content"
                          << result.html_content // Storing full content
                          << "metadata" << finalize;

    // Insert the document
    auto insert_result = collection.insert_one(doc.view());

    if (insert_result &&
        insert_result->inserted_id().type() == bsoncxx::type::k_oid) {
      std::string inserted_id =
          insert_result->inserted_id().get_oid().value.to_string();
      std::cout << "Stored download result for URL: " << result.url
                << " with ID: " << inserted_id << std::endl;
      return true;
    } else {
      std::cerr << "Failed to store download result for URL: " << result.url
                << std::endl;
      return false;
    }
  } catch (const mongocxx::exception &e) {
    std::cerr << "MongoDB exception while storing download result: " << e.what()
              << std::endl;
    return false;
  } catch (const std::exception &e) {
    std::cerr << "General exception while storing download result: " << e.what()
              << std::endl;
    return false;
  }
}
