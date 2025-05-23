#include "../include/ResolverHelper.h"
#include "../include/UrlManager.h"
#include <bsoncxx/array/view.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/types/bson_value/view.hpp>
#include <iostream>
#include <mongocxx/collection.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mutex>
#include <ostream>
#include <set>
#include <string>
#define BATCH_SIZE = 100

// i define collection map here , its better to iterate over map rather then
// using find in mongo
// there is 2 main approuch for updating map , eather i should re read database
// , or i can change it manualy after first read , which as you may geuss i
// chose second method
// ofter changin schema , this map is useless , i change it to set
// std::unordered_map<std::string, std::string> collection_map;
// bool UrlManager::allowed(const std::string &url) {
//  std::cout << "[urlmanager][allowed] this is my url " << url << std::endl;
//  if (!_allowedUrls.empty()) {
//    auto base_url = resolverHelper.extractBaseUrl(url);
//    auto it = find(_allowedUrls.begin(), _allowedUrls.end(), base_url);
//    if (it != _allowedUrls.end()) {
//      std::cout << "[urlmanager][allowed] this url is accepted  " << it.base()
//                << std::endl;
//      return true;
//    }
//    return false;
//  }
//  return true;
//}
bool UrlManager::allowed(const std::string &url) {
  std::cout << "[urlmanager][allowed] this is my url: " << url << std::endl;
  if (!_allowedUrls.empty()) {
    auto base_url = resolverHelper.extractBaseUrl(url);
    std::cout << "[urlmanager][allowed] extracted base url: " << base_url
              << std::endl;

    for (const auto &allowed : _allowedUrls) {
      if (base_url == allowed ||
          (base_url.length() > allowed.length() &&
           base_url.compare(base_url.length() - allowed.length(),
                            allowed.length(), allowed) == 0 &&
           base_url[base_url.length() - allowed.length() - 1] == '.')) {
        std::cout << "[urlmanager][allowed] this url is accepted: " << allowed
                  << std::endl;
        return true;
      }
    }
    std::cout << "[urlmanager][allowed] this url is NOT accepted" << std::endl;
    return false;
  }
  return true; // No allowed list means everything is allowed
}

bool UrlManager::sortingUrls(const std::string &url) {
  if (!allowed(url))
    return false;

  auto client = pool_.acquire();
  auto database = (*client)[database_name_];
  if (!connectionValidator(*client)) {
    std::cerr << "--Connection is not established!" << std::endl;
    return false;
  }

  auto [resolverSuccess, resolverGroupId] =
      resolverHelper.processDomain(database, url);
  if (!resolverSuccess) { // Fixed condition
    std::cerr << "Domain processing failed for URL: " << url << std::endl;
    return false;
  }

  try {
    std::unique_lock<std::mutex> lock(_mutex);
    auto collection = getCollection();

    // Check for duplicate URL (simplified)
    auto duplicate_filter = bsoncxx::builder::stream::document{}
                            << "url" << url
                            << bsoncxx::builder::stream::finalize;
    if (collection.find_one(duplicate_filter.view())) {
      std::cerr << "Duplicate URL detected: " << url << std::endl;
      return false;
    }

    auto batch_filter = bsoncxx::builder::stream::document{}
                        << "base_url" << resolverGroupId
                        << bsoncxx::builder::stream::finalize;
    auto cursor = collection.find(batch_filter.view());

    int batch_id = 0;
    int url_count = 0;
    for (const auto &doc : cursor) {
      int current_batch_id = doc["batch_id"].get_int32().value;
      if (current_batch_id > batch_id) {
        batch_id = current_batch_id;
        url_count = 1;
      } else if (current_batch_id == batch_id) {
        url_count++;
      }
    }

    if (url_count >= 100) {
      batch_id++;
    }

    // Insert new document
    auto url_doc = bsoncxx::builder::stream::document{}
                   << "base_url" << resolverGroupId << "original_domain"
                   << resolverHelper.extractBaseUrl(url) << "batch_id"
                   << batch_id << "url" << url << "hasBeenRead" << false
                   << bsoncxx::builder::stream::finalize;

    if (auto result = collection.insert_one(url_doc.view())) {
      updateMap(collection_map, resolverGroupId);
      std::cout << "Inserted URL into group " << resolverGroupId << ", batch "
                << batch_id << ": " << url << std::endl;
      return true;
    }

    std::cerr << "Failed to insert URL" << std::endl;
    return false;

  } catch (const mongocxx::exception &e) {
    std::cerr << "Database exception: " << e.what() << std::endl;
    return false;
  }
}
bool UrlManager::sortingUrls(std::vector<std::string> urls) {
  for (const auto it : urls) {
  }
  auto client = pool_.acquire();
  auto database = (*client)[database_name_];
  if (!connectionValidator(*client)) {
    std::cerr << "--Connection is not established!" << std::endl;
    return false;
  }
  bool all_success = true;
  for (const std::string &url : urls) {
    if (url.empty()) {
      std::cerr << "Skipping empty URL." << std::endl;
      continue;
    }
    if (!sortingUrls(url)) {
      all_success = false;
    }
  }
  return all_success;
}
Result_read UrlManager::getUrl(std::string domain) {
  auto client = pool_.acquire();
  auto database = (*client)[database_name_];
  if (!connectionValidator(*client)) {
    std::cerr << "--Connection is not established!" << std::endl;
    return Result_read{false, "database connectoin failed" + domain};
  }
  try {
    std::unique_lock<std::mutex> lock(_mutex);
    auto collection = database["urls"];

    // Find the unread URL with the lowest batch_id (earliest batches first)
    auto filter = bsoncxx::builder::stream::document{}
                  << "base_url" << domain << "hasBeenRead" << false
                  << bsoncxx::builder::stream::finalize;

    auto options = mongocxx::options::find{};
    options.sort(bsoncxx::builder::stream::document{}
                 << "batch_id" << 1 // Smallest batch_id first
                 << bsoncxx::builder::stream::finalize);
    options.limit(1);

    auto cursor = collection.find(filter.view(), options);

    if (cursor.begin() == cursor.end()) {
      return Result_read{false, "No unread URLs found for domain: " + domain};
    }

    auto url_doc = *cursor.begin();
    std::string url{url_doc["url"].get_string().value};

    // Mark it as read
    bsoncxx::builder::stream::document update_filter;
    update_filter << "_id" << url_doc["_id"].get_oid().value;

    bsoncxx::builder::stream::document update_op;
    update_op << "$set" << bsoncxx::builder::stream::open_document
              << "hasBeenRead" << true
              << bsoncxx::builder::stream::close_document;

    auto update_result =
        collection.update_one(update_filter.view(), update_op.view());

    lock.unlock();
    if (update_result && update_result->modified_count() > 0) {
      std::cout << "URL retrieved and marked as read: " << url << std::endl;
      return Result_read{true, url};
    } else {
      return Result_read{false, "Failed to mark URL as read: " + url};
    }

  } catch (const mongocxx::exception &e) {
    return Result_read{false, "Error retrieving URL from collection: " +
                                  domain + " - " + e.what()};
  }
}
Bulk_Read UrlManager::getUrl(std::vector<std::string> domains) {
  Bulk_Read results;
  for (const std::string &domain : domains) {
    std::cout << "[UrlManager][getUrl] here is domain  " << domain << std::endl;
    Result_read result = getUrl(domain);
    if (result.status) {
      results.status = true;
    }
    results.result_read.push_back(result);
  }
  if (results.status)
    return results;
  return Bulk_Read();
}
void UrlManager::updateMap(std::set<std::string> &target, std::string key) {
  std::cout << "trying to update map -----------\n";
  if (!map_initiated)
    return;
  else if (target.find(key) == target.end()) {
    target.insert(key);
    map_updated = true;
  }
}
// if i return back to map
std::set<std::string> UrlManager::getBaseUrls() {
  auto client = pool_.acquire();
  auto database = (*client)[database_name_];
  std::set<std::string> temp_map;
  try {
    std::unique_lock<std::mutex> lock(_mutex);
    auto collection = database["urls"];

    bsoncxx::builder::stream::document filter_builder;
    auto filter = filter_builder << bsoncxx::builder::stream::finalize;

    auto cursor = collection.distinct("base_url", filter.view());

    // The cursor contains a single document with a 'values' array
    for (auto &&doc : cursor) {
      // Access the 'values' field
      auto values_element = doc["values"];
      if (!values_element || values_element.type() != bsoncxx::type::k_array) {
        std::cerr << "Expected 'values' field of type array." << std::endl;
        continue;
      }

      // Create a view over the 'values' array
      bsoncxx::array::view values_array = values_element.get_array().value;

      for (const auto &val : values_array) {
        if (val.type() == bsoncxx::type::k_string) {
          std::string base_url(val.get_string().value.data(),
                               val.get_string().value.size());
          temp_map.insert(base_url);
        } else {
          std::cerr << "Unexpected BSON type in values array: "
                    << static_cast<int>(val.type()) << std::endl;
        }
      }
    }
    lock.unlock();
    if (!temp_map.empty()) {
      std::cout << "Base URLs loaded successfully. Found: " << temp_map.size()
                << " domains." << std::endl;
      map_initiated = true;
    }
  } catch (const mongocxx::exception &e) {
    std::cerr << "An error occurred while fetching base URLs: " << e.what()
              << std::endl;
  }

  for (const auto &url : temp_map) {
    std::cout << url << std::endl;
  }

  return temp_map;
}
std::unordered_map<std::string, std::string> UrlManager::getCollectionNames() {
  std::unordered_map<std::string, std::string> collection_map;
  auto client = pool_.acquire();
  auto database = (*client)[database_name_];
  try {
    auto collection = database["domain_groups"];
    auto cursor = collection.distinct("base_url", bsoncxx::document::view{});

    for (const auto &doc : cursor) {
      // The distinct values are returned as documents with a single "value"
      // field
      auto value_field = doc["value"];
      if (value_field && value_field.type() == bsoncxx::type::k_string) {
        std::string base_url(value_field.get_string().value);

        // Find the earliest (or any) document for each base_url to get _id
        auto filter = bsoncxx::builder::stream::document{}
                      << "base_url" << base_url
                      << bsoncxx::builder::stream::finalize;

        auto maybe_doc = collection.find_one(filter.view());

        if (maybe_doc) {
          auto doc = *maybe_doc;
          auto id_element = doc["_id"];
          if (id_element && id_element.type() == bsoncxx::type::k_oid) {
            collection_map[base_url] = id_element.get_oid().value.to_string();
          }
        }
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
Result_read UrlManager::getUrl(std::string domain) {
  if (!is_connected_) {
    std::cerr << "Connection is not established!" << std::endl;
    return Result_read{false, "Connection is not established!"};
  }

  try {
    // agian i like this : auto results =
    // collection.find(make_document(kvp("<field name>", "<value>"))); but
seems
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
/*
smatchd::unordered_map<std::string, std::string>
UrlManager::getCollectionNames() { std::unordered_map<std::string,
std::string> collection_map;

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

/*
std::unordered_map<std::string, std::string> UrlManager::getCollectionNames()
{ try { auto collection_names = database_["collection_names"]; auto cursor =
collection_names.find({});

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
                                        <<
bsoncxx::builder::stream::finalize);

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
/* sorting url with old schema :
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
        batch_doc << "base_url" << base_url << "batch_id" << batch_id <<
"urls"
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
*/
