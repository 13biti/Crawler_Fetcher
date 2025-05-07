#include "../include/CurlDownloder.h"
#include <iostream>

void PrintDownloadResult(const DownloadResult &result) {
  std::cout << "=== Download Result ===\n";
  std::cout << "URL: " << result.url << "\n";
  std::cout << "Status: " << (result.result == CURLE_OK ? "Success" : "Failed")
            << "\n";
  if (result.result != CURLE_OK) {
    std::cout << "Error: " << result.error_message << "\n";
  }
  std::cout << "HTTP Code: " << result.http_code << "\n";
  std::cout << "Timestamp: " << result.timestamp << "\n";
  std::cout << "Content Length: " << result.html_content.length() << " bytes\n";
  std::cout << "First 100 chars: ";
  if (!result.html_content.empty()) {
    std::cout << result.html_content.substr(0, 100) << "...";
  } else {
    std::cout << "[Empty]";
  }
  std::cout << "\n\n";
}

void TestSingleDownload() {
  std::cout << "=== Testing Single Download ===\n";
  Downloader downloader;

  // Test with various URLs
  std::vector<std::string> testUrls = {
      "https://www.example.com",                      // Standard website
      "https://jsonplaceholder.typicode.com/posts/1", // API endpoint
      "https://www.google.com",                       // Another website
      "https://nonexistentdomain.xyz",                // Should fail
      "https://www.example.com/invalidpath"           // Should return 404
  };

  for (const auto &url : testUrls) {
    std::cout << "Downloading: " << url << "\n";
    DownloadResult result = downloader.DownloadSingle(url);
    PrintDownloadResult(result);
  }
}

void TestMultipleDownloads() {
  std::cout << "\n=== Testing Multiple Downloads (Parallel) ===\n";
  Downloader downloader;

  std::vector<std::string> testUrls = {
      "https://www.example.com",
      "https://jsonplaceholder.typicode.com/posts/1",
      "https://www.google.com",
      "https://www.github.com",
      "https://nonexistentdomain.xyz",
      "https://www.example.com/invalidpath",
      "https://httpbin.org/get",
      "https://www.wikipedia.org"};

  // Test with different parallel levels
  for (int parallel : {2, 4, 6}) {
    std::cout << "\nTesting with max_parallel = " << parallel << "\n";
    auto results = downloader.DownloadMultiple(testUrls, parallel);

    for (const auto &result : results) {
      std::cout << (result.result == CURLE_OK ? "[OK] " : "[FAIL] ")
                << result.url << " (" << result.http_code << ")\n";
    }

    // Print details for the first and last results
    std::cout << "\nFirst result details:\n";
    PrintDownloadResult(results.front());

    std::cout << "Last result details:\n";
    PrintDownloadResult(results.back());
  }
}

int main() {
  try {
    TestSingleDownload();
    TestMultipleDownloads();
    std::cout << "All tests completed.\n";
  } catch (const std::exception &e) {
    std::cerr << "Test failed with exception: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
