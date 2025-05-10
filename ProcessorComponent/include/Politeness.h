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

  int getIdByNodeName(std::string nodeName);
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

  // there are tree main stage , ether link is downloaded ,which ack the job ,
  // or there is no link for this domainGroup which nack happen , and suspend
  // happening when domainGroup represent is in the queue
  // assuming the each entity in queue last of 10 min , ( but still dont know
  // how to setpu this )
  void AckJob(int jobId) {
    std::cout << "[updat] updating this " << jobId << std::endl;
    displayStrHeap();
    updateStrJob(jobId, getCurrentTimestampInMilliseconds() +
                            secondsToMilliseconds(timerVal));
  }
  void SuspendJob(int jobId) {
    std::cout << "[suspend] , suspending this " << jobId << std::endl;
    updateStrJob(jobId, getCurrentTimestampInMilliseconds() +
                            secondsToMilliseconds(600));
  }
  void NAckJob(int jobId) {
    uint64_t currentTime = getCurrentTimestampInMilliseconds();
    uint64_t delay = secondsToMilliseconds(timerVal * 5);
    uint64_t newTime = currentTime + delay;
    std::cout << "NAckJob - ID: " << jobId << " | Current Time: " << currentTime
              << " | New Time: " << newTime << " | Delay: " << delay
              << std::endl;
    updateStrJob(jobId, newTime);
  }

private:
  uint64_t timerVal = 10;
  Heap heap;
  StrHeap strHeap;
  std::unordered_map<int, Handle> jobHandles;
  std::unordered_map<int, StrHandle> strJobHandles;
  std::unordered_map<std::string, int> nodeNameToId;
  int nextId = 0;
};
#endif // POLITENESS_H
