#include "../include/PerformHttpRequest.h"
#include <curl/curl.h>
#include <iostream>

size_t HTTPRequest::WriteCallback(void *contents, size_t size, size_t nmemb,
                                  std::string *s) {
  size_t newLength = size * nmemb;
  try {
    s->append((char *)contents, newLength);
  } catch (std::bad_alloc &e) {
    return 0;
  }
  return newLength;
}

HTTPRequest::HttpResponse HTTPRequest::performHttpRequest(
    const std::string &url, const std::string &method, const std::string &data,
    const std::vector<std::string> &headers) {
  CURL *curl;
  CURLcode res;
  std::string response;
  long statusCode = 0;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    if (!data.empty()) {
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    }

    struct curl_slist *curlHeaders = nullptr;
    for (const auto &header : headers) {
      curlHeaders = curl_slist_append(curlHeaders, header.c_str());
    }
    if (curlHeaders) {
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curlHeaders);
    }

    res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
    } else {
      std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res)
                << std::endl;
    }

    if (curlHeaders) {
      curl_slist_free_all(curlHeaders);
    }
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();
  return {response, statusCode};
}
