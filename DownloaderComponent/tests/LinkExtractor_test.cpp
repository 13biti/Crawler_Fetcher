#include "../../ProcessorComponent/include/LinkExtractor.h"
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
  std::cout << "\n";
}

void PrintLinks(const std::vector<std::pair<std::string, std::string>> &links,
                const std::string &title = "Links") {
  std::cout << "=== " << title << " (" << links.size() << ") ===\n";
  for (const auto &[text, url] : links) {
    std::cout << "Text: \"" << (text.empty() ? "[none]" : text) << "\"\n";
    std::cout << "URL: " << url << "\n";
    std::cout << "---\n";
  }
  std::cout << "\n";
}

void TestLinkExtraction(const DownloadResult &result) {
  if (result.result != CURLE_OK || result.html_content.empty()) {
    std::cout
        << "Skipping link extraction - download failed or empty content\n";
    return;
  }

  std::cout << "Extracting redirect links from: " << result.url << "\n";

  auto links =
      LinkExtractor::ExtractRedirectLinks(result.html_content, result.url);

  std::cout << "Found " << links.size() << " redirect links:\n";
  for (const auto &link : links) {
    std::cout << "  " << link << "\n";
  }
  std::cout << "\n";
}

void TestSingleDownloadWithLinkExtraction() {
  std::cout << "=== Testing Single Download with Link Extraction ===\n";
  Downloader downloader;

  // Test with various URLs
  std::vector<std::string> testUrls = {
      "https://www.example.com",                  // Standard website with links
      "https://www.wikipedia.org/wiki/Main_Page", // Page with many links
      "https://news.ycombinator.com",             // Another site with links
      "https://nonexistentdomain.xyz",            // Should fail
      "https://www.example.com/invalidpath"       // Should return 404
  };

  for (const auto &url : testUrls) {
    std::cout << "\n========================================\n";
    std::cout << "Downloading and analyzing: " << url << "\n";
    DownloadResult result = downloader.DownloadSingle(url);
    PrintDownloadResult(result);
    TestLinkExtraction(result);
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

int main() {
  try {
    // Original tests
    TestSingleDownload();
    TestMultipleDownloads();

    // New test with link extraction
    TestSingleDownloadWithLinkExtraction();

    std::cout << "All tests completed.\n";
  } catch (const std::exception &e) {
    std::cerr << "Test failed with exception: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
