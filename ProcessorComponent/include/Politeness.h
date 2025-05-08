#ifndef POLITENESS_H
#define POLITENESS_H
#include <boost/heap/fibonacci_heap.hpp>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
static auto getCurrentTimestampInSeconds = []() -> uint64_t {
  return std::chrono::duration_cast<std::chrono::seconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
};
static uint64_t secondsToMilliseconds(int seconds) {
  return static_cast<uint64_t>(seconds) * 1000;
}

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
  struct JotDto {
    int id;
    std::string base_url;
    bool status;
  };
  struct StrJob {
    int id;
    std::string nodeName;
    uint64_t timestamp;

    bool operator==(const StrJob &other) const { return id == other.id; }

    bool operator<(const StrJob &other) const {
      if (timestamp == other.timestamp)
        return id > other.id;
      return timestamp > other.timestamp;
    }
  };

  template <typename T> struct Comparator {
    bool operator()(const T &a, const T &b) const {
      if (a.timestamp == b.timestamp) {
        return a.id > b.id;
      }
      return a.timestamp > b.timestamp;
    }
  };
  // adding string jobs :
  void addJob(int id, const std::string &nodeName, uint64_t timestamp);
  void addJob(const std::string &nodeName, uint64_t timestamp);
  void addJobs(const std::set<std::string> &nodeNames);
  void deleteStrJob(int id);
  void updateStrJob(int id, uint64_t newTimestamp);
  JotDto getReadyJobStr();
  void displayStrHeap() const;
  void displayStrHeapSorted() const;
  //-----
  void addJob(int id, uint64_t timestamp);
  Job getReadyJob();
  void updateJob(int id, uint64_t newTimestamp);
  void deleteJob(int id);
  void displayHeap() const;
  void displayHeap1() const;
  using Heap =
      boost::heap::fibonacci_heap<Job, boost::heap::compare<Comparator<Job>>>;
  using Handle = Heap::handle_type;

  using StrHeap =
      boost::heap::fibonacci_heap<StrJob,
                                  boost::heap::compare<Comparator<StrJob>>>;
  using StrHandle = StrHeap::handle_type;
  void setTimerVal(int sec) { timerVal = sec; }
  void AckJob(int jobId) {
    updateStrJob(jobId, getCurrentTimestampInSeconds());
  }

  void NAckJob(int jobId) {
    updateStrJob(jobId,
                 (getCurrentTimestampInSeconds() + secondsToMilliseconds(20)));
  }

private:
  uint64_t timerVal = 10;
  Heap heap;
  StrHeap strHeap;
  std::unordered_map<int, Handle> jobHandles;
  std::unordered_map<int, StrHandle> strJobHandles;
  int nextId = 0;
};
#endif // POLITENESS_H
