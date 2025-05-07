// MongoDB.h
#pragma once
#include <mongocxx/instance.hpp>

struct MongoDB {
  static mongocxx::instance &getInstance() {
    static mongocxx::instance instance;
    return instance;
  }
};
