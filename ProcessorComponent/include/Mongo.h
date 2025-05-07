#pragma once
#include <mongocxx/instance.hpp>

class MongoDB {
public:
  static MongoDB &getInstance() {         // Add 'static' here
    static mongocxx::instance instance{}; // Ensures single initialization
    static MongoDB singleton;
    return singleton;
  }

  // Delete copy/move constructors to enforce singleton
  MongoDB(const MongoDB &) = delete;
  MongoDB &operator=(const MongoDB &) = delete;

private:
  MongoDB() = default; // Private constructor
};
