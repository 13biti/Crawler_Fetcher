#include "../include/MongoHandler.h"
struct OperationResult {
    bool success;
    std::string message;
    std::vector<std::string> data; // Or any other type of data you want to return
};

class MongoDBHandler {
public:
    MongoDBHandler(const std::string& uri, const std::string& dbName);

    template <typename T>
    OperationResult storeModel(const std::string& collectionName, const T& model);

    template <typename T>
    OperationResult findModels(const std::string& collectionName, const std::string& queryJson);

private:
    mongocxx::instance instance;
    mongocxx::client client;
    mongocxx::database db;
};

MongoDBHandler::MongoDBHandler(const std::string& uri, const std::string& dbName)
    : instance{}, client{mongocxx::uri{uri}}, db{client[dbName]} {}

template <typename T>
OperationResult MongoDBHandler::storeModel(const std::string& collectionName, const T& model) {
    try {
        auto collection = db[collectionName];
        auto doc = bsoncxx::from_json(model.toJson());
        collection.insert_one(doc.view());

        std::cout << "Successfully stored model in collection: " << collectionName << std::endl;
        return {true, "Model stored successfully", {}};
    } catch (const std::exception& e) {
        std::cerr << "Error storing model: " << e.what() << std::endl;
        return {false, e.what(), {}};
    }
}

template <typename T>
OperationResult MongoDBHandler::findModels(const std::string& collectionName, const std::string& queryJson) {
    try {
        auto collection = db[collectionName];
        auto query = bsoncxx::from_json(queryJson);
        auto cursor = collection.find(query.view());

        std::vector<std::string> models;
        for (auto&& doc : cursor) {
            std::string json = bsoncxx::to_json(doc);
            models.push_back(json);
        }

        std::cout << "Successfully retrieved " << models.size() << " models from collection: " << collectionName << std::endl;
        return {true, "Models retrieved successfully", models};
    } catch (const std::exception& e) {
        std::cerr << "Error finding models: " << e.what() << std::endl;
        return {false, e.what(), {}};
    }
}
