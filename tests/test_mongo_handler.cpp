// this is wroten by ai , still cannot write test , i write like half of it
#include "../include/MongoHandler.h"
#include <gtest/gtest.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

class BaseModel {
public:
  virtual ~BaseModel() = default;
  virtual std::string toJson() const = 0; // Convert model to JSON (for MongoDB)
};

using json = nlohmann::json;
class User : public BaseModel {
public:
  User(const std::string &name = "", int age = 0, const std::string &email = "")
      : name(name), age(age), email(email) {}

  std::string toJson() const override {
    std::ostringstream oss;
    oss << "{"
        << "\"name\": \"" << name << "\", "
        << "\"age\": " << age << ", "
        << "\"email\": \"" << email << "\""
        << "}";
    return oss.str();
  }

  static User fromJson(const std::string &string_json) {
    try {
      json data = json::parse(string_json);
      std::string name = data["name"];
      int age = data["age"];
      std::string email = data["email"];
      return User(name, age, email);
    } catch (const json::parse_error &e) {
      std::cerr << "JSON parsing error: " << e.what() << std::endl;
    } catch (const json::type_error &e) {
      std::cerr << "Type error: " << e.what() << std::endl;
    }
    return User("", 0, " ");
  }

  // Overload equality operator for testing
  bool operator==(const User &other) const {
    return name == other.name && age == other.age && email == other.email;
  }

private:
  std::string name;
  int age;
  std::string email;
};

// Test Configuration
const std::string DATABASE_NAME = "testDb";
const std::string COLLECTION_NAME = "testingColl";
const std::string URI = "mongodb://localhost:27017/";

// Test fixture for MongoDBHandler
class MongoDBHandlerTest : public ::testing::Test {
protected:
  MongoDBHandler *mongoDBHandler;

  void SetUp() override {
    mongoDBHandler = new MongoDBHandler(URI, DATABASE_NAME);
  }

  void TearDown() override { delete mongoDBHandler; }
};

// Test: Store a model in MongoDB
TEST_F(MongoDBHandlerTest, StoreModel) {
  User user("John Doe", 30, "john.doe@example.com");

  // Store the user in MongoDB
  OperationResult result = mongoDBHandler->storeModel(COLLECTION_NAME, user);

  // Check if the operation was successful
  EXPECT_TRUE(result.success);
  EXPECT_EQ(result.message, "Model stored successfully");

  // Log the result
  std::cout << "StoreModel Test: " << result.message << std::endl;
}

// Test: Find models in MongoDB
TEST_F(MongoDBHandlerTest, FindModels) {
  // Insert a test user into the collection
  User testUser("Jane Doe", 25, "jane.doe@example.com");
  mongoDBHandler->storeModel(COLLECTION_NAME, testUser);

  // Define a query to find users with age greater than 20
  std::string queryJson = R"({"age": {"$gt": 20}})";

  // Find models matching the query
  OperationResult result =
      mongoDBHandler->findModels<User>(COLLECTION_NAME, queryJson);

  // Check if the operation was successful
  EXPECT_TRUE(result.success);
  EXPECT_EQ(result.message, "Models retrieved successfully");

  // Deserialize the JSON results into User objects
  std::vector<User> users;
  for (const auto &json : result.data) {
    users.push_back(User::fromJson(json));
  }

  // Check if the test user was found
  bool found = false;
  for (const auto &user : users) {
    if (user == testUser) {
      found = true;
      break;
    }
  }

  EXPECT_TRUE(found);

  // Log the result
  std::cout << "FindModels Test: Found " << users.size() << " users."
            << std::endl;
}
