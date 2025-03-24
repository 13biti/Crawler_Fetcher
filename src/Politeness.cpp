#include "../include/Politeness.h"

void Politeness::addJob(int id, uint64_t timestamp) {

  Job job{id, timestamp};
  auto handle = heap.push(job);
  jobHandles[id] = handle;
}

Politeness::Job Politeness::getReadyJob() {
  if (heap.empty()) {
    throw std::runtime_error("Heap is empty");
  }
  return heap.top();
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
