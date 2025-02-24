// as always this is not mine !! :)
#include "../include/UrlManager.h"
#include <iostream>
#include <vector>

int main() {
  // Create an instance of UrlManager
  UrlManager urlManager;

  // Test sortingUrls (single URL)
  std::string singleUrl = "https://example.com";
  if (urlManager.sortingUrls(singleUrl)) {
    std::cout << "Successfully inserted URL: " << singleUrl << std::endl;
  } else {
    std::cerr << "Failed to insert URL: " << singleUrl << std::endl;
  }

  // Test sortingUrls (batch URLs)
  std::vector<std::string> urls = {
      "https://example.org", "https://example.net",
      "https://invalid-url" // This should fail due to invalid format
  };
  if (urlManager.sortingUrls(urls)) {
    std::cout << "Successfully inserted all URLs." << std::endl;
  } else {
    std::cerr << "Failed to insert some URLs." << std::endl;
  }

  // Test getUrl (single domain)
  std::string domain = "example.com";
  Result_read singleUrlResult = urlManager.getUrl(domain);
  if (singleUrlResult.status) {
    std::cout << "URL from " << domain << ": " << singleUrlResult.message
              << std::endl;
  } else {
    std::cerr << "Error: " << singleUrlResult.message << std::endl;
  }

  // Test getUrl (multiple domains)
  std::vector<std::string> domains = {"example.com", "example.org",
                                      "example.net"};
  std::vector<Result_read> multipleUrlsResult = urlManager.getUrl(domains);
  for (const auto &result : multipleUrlsResult) {
    if (result.status) {
      std::cout << "URL: " << result.message << std::endl;
    } else {
      std::cerr << "Error: " << result.message << std::endl;
    }
  }

  return EXIT_SUCCESS;
}
