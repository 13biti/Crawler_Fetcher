#include <iostream>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <sstream>
#include <string>

// here i try to have something like what we have in dotnet , Since this
// mongo stores every record, I tried to create a template that allows me to
// add records to it. In general, I'm probably doing it wrong, but I'm dealing
// with it better this way.
class BaseModel {
public:
  virtual ~BaseModel() = default;
  virtual std::string toJson() const = 0; // Convert model to JSON (for MongoDB)
};

using json = nlohmann::json;
class User : public BaseModel {
public:
  User(const std::string &name, int age, const std::string &email)
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

private:
  std::string name;
  int age;
  std::string email;
};
