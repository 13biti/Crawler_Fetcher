#include "../include/DownloadResultStorage.h"
#include <bsoncxx/builder/basic/document.hpp>
#include <iostream>
#include <nlohmann/json.hpp>

// Use the basic builder's kvp
using bsoncxx::builder::basic::kvp;

bool DownloadResultStorage::storeDownloadResult(const result &download_result) {
  auto client = pool_.acquire();
  auto database = client[database_name_];
  try {
    auto collection = database["DownloadedContent"];

    // Build the document using the basic builder
    auto doc = bsoncxx::builder::basic::document{};
    doc.append(
        kvp("url", download_result.url),
        kvp("timestamp", download_result.timestamp),
        kvp("content_length",
            static_cast<int64_t>(download_result.html_content_base64.length())),
        kvp("html_content_base64",
            download_result.html_content_base64), // Store as base64
        kvp("status", download_result.status));

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
