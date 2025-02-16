#include <sstream>
#include <string>

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

  static User fromJson(const std::string &json) {
    // Parse JSON (you can use a library like nlohmann/json for better parsing)
    // For simplicity, we'll assume the JSON is well-formed and extract fields
    // manually.
    size_t nameStart = json.find("\"name\": \"") + 9;
    size_t nameEnd = json.find("\"", nameStart);
    std::string name = json.substr(nameStart, nameEnd - nameStart);

    size_t ageStart = json.find("\"age\": ") + 7;
    size_t ageEnd = json.find(",", ageStart);
    int age = std::stoi(json.substr(ageStart, ageEnd - ageStart));

    size_t emailStart = json.find("\"email\": \"") + 10;
    size_t emailEnd = json.find("\"", emailStart);
    std::string email = json.substr(emailStart, emailEnd - emailStart);

    return User(name, age, email);
  }

private:
  std::string name;
  int age;
  std::string email;
};
