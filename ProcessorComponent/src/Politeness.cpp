#include "../include/Politeness.h"
#include <bits/types/struct_timeval.h>
#include <string>

void Politeness::addJob(int id, uint64_t timestamp) {
  Job job{id, timestamp};
  auto handle = heap.push(job);
  jobHandles[id] = handle;
}

// string jobs ------------------------------------------------- :
int Politeness::getIdByNodeName(std::string nodeName) {
  auto it = nodeNameToId.find(nodeName);
  if (it != nodeNameToId.end())
    return it->second;
  return -1;
}
void Politeness::addJob(int id, const std::string &nodeName,
                        uint64_t timestamp) {
  if (nodeNameToId.find(nodeName) != nodeNameToId.end()) {
    // Job already exists, don't insert
    return;
  }
  StrJob job{id, nodeName, timestamp};
  auto handle = strHeap.push(job);
  strJobHandles[id] = handle;
  nodeNameToId[nodeName] = id;
}

void Politeness::addJob(const std::string &nodeName, uint64_t timestamp) {
  if (nodeNameToId.find(nodeName) != nodeNameToId.end()) {
    return;
  }
  int id = nextId++;
  addJob(id, nodeName, timestamp);
}

void Politeness::addJobs(const std::set<std::string> &nodeNames) {
  for (const auto &name : nodeNames) {
    addJob(name, getCurrentTimestampInMilliseconds());
  }
}
void Politeness::deleteStrJob(int id) {
  auto it = strJobHandles.find(id);
  if (it != strJobHandles.end()) {
    strHeap.erase(it->second);
    strJobHandles.erase(it);
  } else {
    throw std::runtime_error("StrJob not found");
  }
}
void Politeness::updateStrJob(int id, uint64_t newTimestamp) {
  auto it = strJobHandles.find(id);
  if (it != strJobHandles.end()) {
    StrJob updatedJob = *(it->second);
    updatedJob.timestamp = newTimestamp;
    strHeap.update(it->second, updatedJob); // Use built-in update
  } else {
    throw std::runtime_error("Job not found");
  }
}
Politeness::JotDto Politeness::getReadyJobStr() {
  if (strHeap.empty()) {
    throw std::runtime_error("Heap is empty");
  }
  StrJob readyOne = strHeap.top();
  bool isWaited = getCurrentTimestampInMilliseconds() >= readyOne.timestamp;

  if (isWaited)
    return Politeness::JotDto{readyOne.id, readyOne.nodeName, true};
  return Politeness::JotDto{0, "", false};
}

void Politeness::displayStrHeap() const {
  for (const auto &job : strHeap) {
    std::cout << "StrJob ID: " << job.id << " | Node: " << job.nodeName
              << " | Timestamp: " << job.timestamp << std::endl;
  }
}
void Politeness::displayStrHeapSorted() const {
  std::vector<StrJob> jobs;
  for (const auto &handle : strJobHandles) {
    jobs.push_back(*handle.second);
  }

  std::sort(jobs.begin(), jobs.end(), [](const StrJob &a, const StrJob &b) {
    return a.timestamp < b.timestamp;
  });

  for (const auto &job : jobs) {
    std::cout << "StrJob ID: " << job.id << " | Node: " << job.nodeName
              << " | Timestamp: " << job.timestamp << std::endl;
  }
}

// -----------
Politeness::Job Politeness::getReadyJob() {
  if (heap.empty()) {
    throw std::runtime_error("Heap is empty");
  }
  Job readyOne = heap.top();
  if (readyOne.timestamp + secondsToMilliseconds(timerVal) >
      getCurrentTimestampInMilliseconds())
    return Politeness::Job();
  return readyOne;
}

void Politeness::updateJob(int id, uint64_t newTimestamp) {
  auto it = jobHandles.find(id);
  if (it != jobHandles.end()) {
    Job updatedJob = *(it->second);
    updatedJob.timestamp = newTimestamp;
    heap.update(it->second, updatedJob);
  } else {
    throw std::runtime_error("Job not found");
  }
}

void Politeness::deleteJob(int id) {
  auto it = jobHandles.find(id);
  if (it != jobHandles.end()) {
    heap.erase(it->second);
    jobHandles.erase(it);
  } else {
    throw std::runtime_error("Job not found");
  }
}
void Politeness::displayHeap() const {
  for (const auto &job : heap) {
    std::cout << "Job ID: " << job.id << " | Timestamp: " << job.timestamp
              << std::endl;
  }
}
void Politeness::displayHeap1() const {
  std::vector<Job> jobs;
  for (const auto &handle : jobHandles) {
    jobs.push_back(*handle.second);
  }
  std::sort(jobs.begin(), jobs.end(), [](const Job &a, const Job &b) {
    return a.timestamp < b.timestamp;
  });
  for (const auto &job : jobs) {
    std::cout << "Job ID: " << job.id << " | Timestamp: " << job.timestamp
              << std::endl;
  }
}
