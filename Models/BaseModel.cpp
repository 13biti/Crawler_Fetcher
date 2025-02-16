#include <string>

class BaseModel {
public:
  virtual ~BaseModel() = default;
  virtual std::string toJson() const = 0; // Convert model to JSON (for MongoDB)
};
