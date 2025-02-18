#ifndef MONGODBHANDLER_H
#define MONGODBHANDLER_H

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <iostream>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <vector>

class MongoDBHandler {
public:
  MongoDBHandler(const std::string &uri, const std::string &dbName);

  template <typename T>
  void storeModel(const std::string &collectionName, const T &model);

  template <typename T>
  std::vector<T> findModels(const std::string &collectionName,
                            const std::string &queryJson = "{}");

private:
  mongocxx::instance instance;
  mongocxx::client client;
  mongocxx::database db;
};
#endif
