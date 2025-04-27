#include "../include/UrlManager.h"
#include <bsoncxx/builder/basic/document.hpp>
#include <iostream>

#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>

#include <mongocxx/exception/exception.hpp>

#include <mongocxx/instance.hpp>

#include <mongocxx/uri.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#define BATCH_SIZE = 100

// i define collection map here , its better to iterate over map rather then
// using find in mongo
// there is 2 main approuch for updating map , eather i should re read database
// , or i can change it manualy after first read , which as you may geuss i
// chose second method
std::unordered_map<std::string, std::string> collection_map;
bool map_initiated = false;
bool map_updated = false;

void UrlManager::retryConnection(int interval_seconds = 10) {
  while (!is_connected_) {
    std::cout << "Retrying connection to MongoDB..." << std::endl;
    connectToMongoDB("mongodb://localhost:27017/", "testDb", "admin");
    std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
  }
}
bool UrlManager::sortingUrls(const std::string &url) {
  if (!is_connected_) {
    std::cerr << "Connection is not established!" << std::endl;
    return false;
  }

  std::smatch result;
  std::regex pattern(R"((?:https?://)?([^/]+))");
  if (std::regex_search(url, result, pattern)) {
    std::string base_url = result.str(1);

    try {
      auto collection = database_["urls"];

      // Find the latest batch_id document for this base_url
      auto cursor = collection.find(bsoncxx::builder::stream::document{}
                                    << "base_url" << base_url
                                    << bsoncxx::builder::stream::finalize);

      int batch_id = 0;
      bsoncxx::document::view latest_batch_doc;
      bool batch_found = false;

      for (const auto &doc : cursor) {
        int current_batch_id = doc["batch_id"].get_int32().value;
        if (current_batch_id > batch_id) {
          batch_id = current_batch_id;
          latest_batch_doc = doc;
          batch_found = true;
        }
      }

      bool create_new_batch = true;
      if (batch_found) {
        auto urls_array = latest_batch_doc["urls"].get_array().value;
        int url_count = std::distance(urls_array.begin(), urls_array.end());

        if (url_count < 100) {
          create_new_batch = false;
        } else {
          batch_id++;
        }
      } else {
        batch_id = 1;
      }

      bsoncxx::builder::stream::document url_doc;
      url_doc << "url" << url << "hasBeenRead" << false;

      if (create_new_batch) {
        bsoncxx::builder::stream::document batch_doc;
        batch_doc << "base_url" << base_url << "batch_id" << batch_id << "urls"
                  << bsoncxx::builder::stream::open_array << url_doc
                  << bsoncxx::builder::stream::close_array;

        auto insert_result = collection.insert_one(batch_doc.view());
        if (insert_result &&
            insert_result->inserted_id().type() == bsoncxx::type::k_oid) {
          std::string inserted_id =
              insert_result->inserted_id().get_oid().value.to_string();

          updateMap(collection_map, base_url,
                    inserted_id); // Pass by reference!

          std::cout << "Created new batch " << batch_id
                    << " and inserted URL: " << url << std::endl;
        }
      } else {
        bsoncxx::builder::stream::document update_query;
        update_query << "base_url" << base_url << "batch_id" << batch_id;

        bsoncxx::builder::stream::document update_op;
        update_op << "$push" << bsoncxx::builder::stream::open_document
                  << "urls" << url_doc
                  << bsoncxx::builder::stream::close_document;

        auto update_result =
            collection.update_one(update_query.view(), update_op.view());

        if (update_result) {
          std::cout << "URL added to batch " << batch_id << ": " << url
                    << std::endl;
        } else {
          std::cerr << "Failed to update batch " << batch_id << " with URL."
                    << std::endl;
          return false;
        }
      }

      return true;

    } catch (const mongocxx::exception &e) {
      std::cerr << "An exception occurred: " << e.what() << std::endl;
      return false;
    }

  } else {
    std::cerr << "Invalid URL format: " << url << std::endl;
    return false;
  }
}
/*
UrlManagerl::sortingUrls(std::string url) {
  if (!is_connected_) {
    std::cerr << "Connection is not established!" << std::endl;
    return false;
  }

  std::smatch result;
  std::regex pattern(R"((?:https?://)?([^/]+))");
  if (std::regex_search(url, result, pattern)) {
    std::string target = result.str(1);
    auto collection = database_[target];

    // Insert the URL into the collection
    try {
      auto insert_result =
          collection.insert_one(bsoncxx::builder::stream::document{}
                                << "url" << url << "hasBeenRead" << false
                                << bsoncxx::builder::stream::finalize);

      if (insert_result) {
        std::cout << "URL inserted successfully: " << url << std::endl;

        auto collection_names =
            database_["collection_names"];
        auto insert_name_result =
            collection_names.insert_one(bsoncxx::builder::stream::document{}
                                        << "collection_name" << target
                                        << bsoncxx::builder::stream::finalize);

        if (insert_name_result) {
          std::cout << "Collection name inserted successfully: " << target
                    << std::endl;
        } else {
          std::cerr << "Failed to insert collection name: " << target
                    << std::endl;
        }

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
*/

bool UrlManager::sortingUrls(std::vector<std::string> urls) {
  if (!is_connected_) {
    std::cerr << "connection is not established !" << std::endl;
    return false;
  }
  bool all_success = true;
  for (const std::string &url : urls) {
    if (url.empty()) {
      std::cerr << "Skipping empty URL." << std::endl;
      continue; // Skip empty URLs
    }
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
    auto collection = database_["urls"];

    // Find the latest batch_id for this domain
    auto sort_order = bsoncxx::builder::stream::document{}
                      << "batch_id" << -1 // Sort in descending order
                      << bsoncxx::builder::stream::finalize;

    auto batch_cursor = collection.find(
        bsoncxx::builder::stream::document{}
            << "base_url" << domain << bsoncxx::builder::stream::finalize,
        mongocxx::options::find{}.sort(sort_order).limit(1));

    if (batch_cursor.begin() == batch_cursor.end()) {
      return Result_read{false, "No batches found for domain: " + domain};
    }

    auto latest_batch_doc = *batch_cursor.begin();
    auto urls_array = latest_batch_doc["urls"].get_array().value;

    // Find the first unread URL in the latest batch
    for (const auto &url_doc : urls_array) {
      auto url_obj = url_doc.get_document().value;
      if (!url_obj["hasBeenRead"].get_bool().value) {
        std::string url = url_obj["url"].get_string().value;

        // Mark the URL as read
        bsoncxx::builder::stream::document update_query;
        update_query << "base_url" << domain << "batch_id"
                     << latest_batch_doc["batch_id"].get_int32().value
                     << "urls.url" << url;

        bsoncxx::builder::stream::document update_op;
        update_op << "$set" << bsoncxx::builder::stream::open_document
                  << "urls.$.hasBeenRead" << true
                  << bsoncxx::builder::stream::close_document;

        auto update_result =
            collection.update_one(update_query.view(), update_op.view());

        if (update_result && update_result->modified_count() > 0) {
          std::cout << "URL retrieved and marked as read: " << url << std::endl;
          return Result_read{true, url};
        } else {
          return Result_read{false, "Failed to mark URL as read: " + url};
        }
      }
    }

    // No unread URL found
    return Result_read{false, "No unread URLs found in the latest batch for: " +
                                  domain};

  } catch (const mongocxx::exception &e) {
    return Result_read{false, "Error retrieving URL from collection: " +
                                  domain + " - " + e.what()};
  }
}

/*
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
*/
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
void UrlManager::updateMap(std::unordered_map<std::string, std::string> &target,
                           std::string key, std::string value) {
  if (map_initiated)
    return;
  else if (target.find(key) == target.end())
    target.insert({key, value});
  else
    target[key] = value;
  map_updated = true;
}
/*
smatchd::unordered_map<std::string, std::string>
UrlManager::getCollectionNames() { std::unordered_map<std::string, std::string>
collection_map;

  try {
    auto collection = database_["urls"];

    mongocxx::pipeline pipeline{};
    pipeline.group(bsoncxx::builder::stream::document{}
                   << "_id" << "$base_url"
                   << bsoncxx::builder::stream::finalize);

    auto cursor = collection.aggregate(pipeline);

    for (const auto &doc : cursor) {
      std::string base_url = doc["_id"].get_utf8().value.to_string();
      collection_map[base_url] = base_url;
    }

    std::cout << "Base URLs loaded successfully. Found: "
              << collection_map.size() << " domains." << std::endl;
  } catch (const mongocxx::exception &e) {
    std::cerr << "An error occurred while fetching base URLs: " << e.what()
              << std::endl;
  }

  return collection_map;
}
*/
std::unordered_map<std::string, std::string> UrlManager::getCollectionNames() {
  std::unordered_map<std::string, std::string> collection_map;

  try {
    auto collection = database_["urls"];

    // Find distinct base_urls
    auto distinct_base_urls =
        collection.distinct("base_url", bsoncxx::document::view{});

    for (const auto &base_url_element : distinct_base_urls) {
      std::string base_url = base_url_element.get_utf8().value.to_string();

      // Find one document for each base_url to get its _id
      bsoncxx::builder::stream::document filter_builder;
      filter_builder << "base_url" << base_url;

      auto maybe_doc = collection.find_one(filter_builder.view());

      if (maybe_doc) {
        auto doc = maybe_doc->view();
        std::string object_id = doc["_id"].get_oid().value.to_string();
        collection_map[base_url] = object_id;
      }
    }

    std::cout << "Base URLs loaded successfully. Found: "
              << collection_map.size() << " domains." << std::endl;
  } catch (const mongocxx::exception &e) {
    std::cerr << "An error occurred while fetching base URLs: " << e.what()
              << std::endl;
  }

  return collection_map;
}

/*
std::unordered_map<std::string, std::string> UrlManager::getCollectionNames() {
  try {
    auto collection_names = database_["collection_names"];
    auto cursor = collection_names.find({});

    for (auto &&doc : cursor) {
      std::string collection_name =
          doc["collection_name"].get_utf8().value.to_string();
      collection_map[collection_name] =
          collection_name; // You could store more info here if needed
    }
    std::cout << "Collection names loaded successfully." << std::endl;
  } catch (const mongocxx::exception &e) {
    std::cerr << "An error occurred while fetching collection names: "
              << e.what() << std::endl;
  }

  return collection_map;
}
*/
