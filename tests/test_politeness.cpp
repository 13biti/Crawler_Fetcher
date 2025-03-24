#include "../include/Politeness.h"
#include <iostream>

void testAllJobsArrived() {
  Politeness scheduler;

  // Add jobs with timestamps in the past
  uint64_t currentTime = getCurrentTimestampInMilliseconds();
  for (int i = 1; i <= 5; ++i) {
    scheduler.addJob(i, currentTime - (i * 1000)); // Timestamps in the past
    std::cout << "Added Job ID: " << i
              << " | Timestamp: " << currentTime - (i * 1000) << std::endl;
  }

  // Retrieve the ready job (should be the one with the earliest timestamp)
  try {
    Politeness::Job readyJob = scheduler.getReadyJob();
    std::cout << "\nReady Job ID: " << readyJob.id
              << " | Timestamp: " << readyJob.timestamp << std::endl;
  } catch (const std::runtime_error &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  // Display the heap
  std::cout << "\nHeap Structure (All Jobs Arrived):" << std::endl;
  scheduler.displayHeap();
}

void testNoJobsArrived() {
  Politeness scheduler;

  // Add jobs with timestamps in the future
  uint64_t currentTime = getCurrentTimestampInMilliseconds();
  for (int i = 1; i <= 5; ++i) {
    scheduler.addJob(i, currentTime + (i * 1000)); // Timestamps in the future
    std::cout << "Added Job ID: " << i
              << " | Timestamp: " << currentTime + (i * 1000) << std::endl;
  }

  // Retrieve the ready job (should be the one with the earliest future
  // timestamp)
  try {
    Politeness::Job readyJob = scheduler.getReadyJob();
    std::cout << "\nReady Job ID: " << readyJob.id
              << " | Timestamp: " << readyJob.timestamp << std::endl;
  } catch (const std::runtime_error &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  // Display the heap
  std::cout << "\nHeap Structure (No Jobs Arrived):" << std::endl;
  scheduler.displayHeap();
}

void testMixedScenario() {
  Politeness scheduler;

  // Add jobs with mixed timestamps (past and future)
  uint64_t currentTime = getCurrentTimestampInMilliseconds();
  scheduler.addJob(1, currentTime - 2000); // Past
  scheduler.addJob(2, currentTime + 1000); // Future
  scheduler.addJob(3, currentTime - 1000); // Past
  scheduler.addJob(4, currentTime + 2000); // Future
  scheduler.addJob(5, currentTime - 3000); // Past

  // Display the heap
  std::cout << "\nHeap Structure (Mixed Scenario):" << std::endl;
  scheduler.displayHeap();

  // Retrieve the ready job (should be the one with the earliest timestamp)
  try {
    Politeness::Job readyJob = scheduler.getReadyJob();
    std::cout << "\nReady Job ID: " << readyJob.id
              << " | Timestamp: " << readyJob.timestamp << std::endl;
  } catch (const std::runtime_error &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  // Update a job to move it to the middle of the heap
  try {
    scheduler.updateJob(
        2, currentTime + 500); // Move job 2 to a slightly earlier future time
    std::cout << "\nUpdated Job ID: 2 | New Timestamp: " << currentTime + 500
              << std::endl;
  } catch (const std::runtime_error &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  // Display the heap after update
  std::cout << "\nHeap Structure After Update:" << std::endl;
  scheduler.displayHeap();
}

void testEdgeCases() {
  Politeness scheduler;

  // Test empty heap
  try {
    Politeness::Job readyJob = scheduler.getReadyJob();
    std::cout << "Ready Job ID: " << readyJob.id
              << " | Timestamp: " << readyJob.timestamp << std::endl;
  } catch (const std::runtime_error &e) {
    std::cerr << "Error (Empty Heap): " << e.what() << std::endl;
  }

  // Test deleting a non-existent job
  try {
    scheduler.deleteJob(99); // Job 99 doesn't exist
  } catch (const std::runtime_error &e) {
    std::cerr << "Error (Delete Non-Existent Job): " << e.what() << std::endl;
  }

  // Test updating a non-existent job
  try {
    scheduler.updateJob(
        99, getCurrentTimestampInMilliseconds()); // Job 99 doesn't exist
  } catch (const std::runtime_error &e) {
    std::cerr << "Error (Update Non-Existent Job): " << e.what() << std::endl;
  }
}

int main() {
  std::cout << "=== Test All Jobs Arrived ===" << std::endl;
  testAllJobsArrived();

  std::cout << "\n=== Test No Jobs Arrived ===" << std::endl;
  testNoJobsArrived();

  std::cout << "\n=== Test Mixed Scenario ===" << std::endl;
  testMixedScenario();

  std::cout << "\n=== Test Edge Cases ===" << std::endl;
  testEdgeCases();

  return 0;
}
