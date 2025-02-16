#include "../include/MongoHandler.h"

MongoDBHandler::MongoDBHandler(const std::string &uri,
                               const std::string &dbName)
    : instance{}, client{mongocxx::uri{uri}}, db{client[dbName]} {}

template <typename T>
void MongoDBHandler::storeModel(const std::string &collectionName,
                                const T &model) {
  auto collection = db[collectionName];
  auto doc = bsoncxx::from_json(model.toJson());
  collection.insert_one(doc.view());
}

template <typename T>
std::vector<T> MongoDBHandler::findModels(const std::string &collectionName,
                                          const std::string &queryJson) {
  auto collection = db[collectionName];
  auto query = bsoncxx::from_json(queryJson);
  auto cursor = collection.find(query.view());

  std::vector<T> models;
  for (auto &&doc : cursor) {
    std::string json = bsoncxx::to_json(doc);
    models.push_back(T::fromJson(json));
  }

  return models;
}
