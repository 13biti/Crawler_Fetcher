#include "../include/DownloadResultStorage.h"
#include <iostream>

using namespace bsoncxx::builder::stream;

void DownloadResultStorage::connectToMongoDB(const std::string &mongo_uri,
                                             const std::string &database_name,
                                             const std::string &client_name) {
  try {
    // Ping the database to check connection
    auto admin_db = client_["admin"];
    auto ping_cmd = document{} << "ping" << 1 << finalize;
    admin_db.run_command(ping_cmd.view());

    database_ = client_[database_name];
    is_connected_ = true;
    std::cout << "Successfully connected to MongoDB: " << mongo_uri
              << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "MongoDB connection error: " << e.what() << std::endl;
    is_connected_ = false;
  }
}

bool DownloadResultStorage::storeDownloadResult(const result &download_result) {
  try {
    auto collection = database_["DownloadedContent"];

    // Build the document using the stream builder
    auto doc = document{};
    doc << "url" << download_result.url << "timestamp"
        << download_result.timestamp << "content_length"
        << static_cast<int64_t>(download_result.html_content.length())
        << "html_content" << download_result.html_content << "status"
        << download_result.status;

    // Insert the document
    auto insert_result = collection.insert_one(doc.view());

    if (insert_result) {
      if (insert_result->inserted_id().type() == bsoncxx::type::k_oid) {
        std::string inserted_id =
            insert_result->inserted_id().get_oid().value.to_string();
        std::cout << "Stored download result for URL: " << download_result.url
                  << " with ID: " << inserted_id << std::endl;
        return true;
      }
    }
    std::cerr << "Failed to store download result for URL: "
              << download_result.url << std::endl;
    return false;

  } catch (const std::exception &e) {
    std::cerr << "Exception while storing download result: " << e.what()
              << std::endl;
    return false;
  }
}
