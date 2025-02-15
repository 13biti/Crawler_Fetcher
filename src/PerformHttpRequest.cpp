#include "include/PerformHttpRequest.h"
#include <cstddef>
#include <stdexcept>

// Callback function to handle response data
size_t HTTPRequest::WriteCallback(void *contents, size_t size, size_t nmemb,
                                  std::string *s) {
  size_t newLength = size * nmemb;
  try {
    s->append((char *)contents, newLength);
  } catch (std::bad_alloc &e) {
    // Handle memory problem
    return 0;
  }
  return newLength;
}

// Perform an HTTP request
std::string HTTPRequest::performHttpRequest(
    const std::string &url, const std::string &method, const std::string &data,
    const std::vector<std::string> &headers) {
  CURL *curl;
  CURLcode res;
  std::string response;

  // Initialize libcurl
  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  if (!curl) {
    curl_global_cleanup();
    throw std::runtime_error("Failed to initialize CURL");
  }

  // Set up the request
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  // Add request data (if any)
  if (!data.empty()) {
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
  }

  // Add headers (if any)
  struct curl_slist *curlHeaders = nullptr;
  for (const auto &header : headers) {
    curlHeaders = curl_slist_append(curlHeaders, header.c_str());
  }
  if (curlHeaders) {
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curlHeaders);
  }

  // Perform the request
  res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res)
              << std::endl;
  }

  // Clean up
  if (curlHeaders) {
    curl_slist_free_all(curlHeaders);
  }
  curl_easy_cleanup(curl);
  curl_global_cleanup();

  return response;
}
