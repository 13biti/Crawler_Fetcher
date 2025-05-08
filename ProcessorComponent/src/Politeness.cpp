#include "../include/Politeness.h"
#include <bits/types/struct_timeval.h>
#include <string>

void Politeness::addJob(int id, uint64_t timestamp) {
  Job job{id, timestamp};
  auto handle = heap.push(job);
  jobHandles[id] = handle;
}

// string jobs ------------------------------------------------- :
void Politeness::addJob(int id, const std::string &nodeName,
                        uint64_t timestamp) {
  StrJob job{id, nodeName, timestamp};
  auto handle = strHeap.push(job);
  strJobHandles[id] = handle;
}

void Politeness::addJob(const std::string &nodeName, uint64_t timestamp) {
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
    strHeap.update(it->second, updatedJob);
  } else {
    throw std::runtime_error("StrJob not found");
  }
}
Politeness::JotDto Politeness::getReadyJobStr() {
  if (strHeap.empty()) {
    throw std::runtime_error("Heap is empty");
  }
  StrJob readyOne = strHeap.top();
  //  std::cout << "readyOne is " << std::to_string(readyOne.timestamp) << " "
  //            << readyOne.nodeName << " timerval "
  //            << std::to_string(secondsToMilliseconds(timerVal))
  //            << " sum of timeval and timestamp "
  //            << std::to_string(readyOne.timestamp +
  //                              secondsToMilliseconds(timerVal))
  //            << " current time "
  //            << std::to_string(getCurrentTimestampInMilliseconds())
  //            << " is it bigger? "
  //            << ((readyOne.timestamp + secondsToMilliseconds(timerVal)) <
  //                        getCurrentTimestampInMilliseconds()
  //                    ? "yes"
  //                    : "no")
  //            << std::endl;
  // lord have mercy i make big mistkae , i wrote this if in reverce !!
  if (readyOne.timestamp + secondsToMilliseconds(timerVal) >
      getCurrentTimestampInMilliseconds())
    return Politeness::JotDto{0, "", false};
  //  std::cout << "change time stemp from "
  //            << std::to_string(readyOne.timestamp) + "to" +
  //                   std::to_string(getCurrentTimestampInMilliseconds())
  //            << std::endl;
  updateStrJob(readyOne.id, getCurrentTimestampInMilliseconds());

  return Politeness::JotDto{readyOne.id, readyOne.nodeName, true};
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
