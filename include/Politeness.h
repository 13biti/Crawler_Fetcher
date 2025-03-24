#ifndef POLITENESS_H
#define POLITENESS_H
#include <boost/heap/fibonacci_heap.hpp>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
static auto getCurrentTimestampInSeconds = []() -> uint64_t {
  return std::chrono::duration_cast<std::chrono::seconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
};

static auto getCurrentTimestampInMilliseconds = []() -> uint64_t {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
};
class Politeness {
public:
  struct Job {
    int id;
    uint64_t timestamp;
  };

  struct Comparator {
    bool operator()(const Job &a, const Job &b) const {
      if (a.timestamp == b.timestamp) {
        return a.id > b.id;
      }
      return a.timestamp > b.timestamp;
    }
  };
  void addJob(int id, uint64_t timestamp);
  Job getReadyJob();
  void updateJob(int id, uint64_t newTimestamp);
  void deleteJob(int id);
  void displayHeap() const;
  void displayHeap1() const;
  using Heap =
      boost::heap::fibonacci_heap<Job, boost::heap::compare<Comparator>>;
  using Handle = Heap::handle_type;

private:
  Heap heap;
  std::unordered_map<int, Handle> jobHandles;
};
#endif // POLITENESS_H
