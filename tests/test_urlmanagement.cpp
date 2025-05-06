// like before , this is not mine !! :))
#include "../include/UrlManager.h"
#include <iostream>
#include <vector>

int main() {
  // Create an instance of UrlManager
  UrlManager urlManager;

  // Test inserting single URL
  std::string singleUrl = "https://example.com";
  if (urlManager.sortingUrls(singleUrl)) {
    std::cout << "✅ Successfully inserted URL: " << singleUrl << std::endl;
  } else {
    std::cerr << "❌ Failed to insert URL: " << singleUrl << std::endl;
  }

  // Test inserting multiple URLs (batch)
  std::vector<std::string> urls = {
      "https://example.org", "https://example.net", "https://example.org/page1",
      "https://example.net/page2",
      "https://invalid-url" // This should fail due to invalid format
  };

  if (urlManager.sortingUrls(urls)) {
    std::cout << "✅ Successfully inserted all valid URLs." << std::endl;
  } else {
    std::cerr << "❌ Failed to insert some URLs." << std::endl;
  }

  // Test getting unread URLs (should return an unread URL)
  std::string domain = "example.com";
  Result_read firstUrlResult = urlManager.getUrl(domain);
  if (firstUrlResult.status) {
    std::cout << "🔍 First unread URL from " << domain << ": "
              << firstUrlResult.message << std::endl;
  } else {
    std::cerr << "⚠️ Error: " << firstUrlResult.message << std::endl;
  }

  // Try getting another unread URL from the same domain (should return next
  // unread URL)
  Result_read secondUrlResult = urlManager.getUrl(domain);
  if (secondUrlResult.status) {
    std::cout << "🔍 Second unread URL from " << domain << ": "
              << secondUrlResult.message << std::endl;
  } else {
    std::cerr << "⚠️ No unread URLs left in " << domain << std::endl;
  }

  // Try getting from a different domain (should work for unread URLs)
  std::string anotherDomain = "example.org";
  Result_read anotherDomainUrl = urlManager.getUrl(anotherDomain);
  if (anotherDomainUrl.status) {
    std::cout << "🔍 Unread URL from " << anotherDomain << ": "
              << anotherDomainUrl.message << std::endl;
  } else {
    std::cerr << "⚠️ No unread URLs in " << anotherDomain << std::endl;
  }

  // Try fetching from example.com again (should return no URLs if marking
  // works)
  Result_read finalCheck = urlManager.getUrl(domain);
  if (!finalCheck.status) {
    std::cout << "✅ Confirmed that no unread URLs remain in " << domain
              << std::endl;
  } else {
    std::cerr << "❌ Error: Expected no unread URLs, but got: "
              << finalCheck.message << std::endl;
  }

  return EXIT_SUCCESS;
}
