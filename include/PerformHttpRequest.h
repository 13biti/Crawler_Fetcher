#ifndef PERFORMHTTPREQUEST_H
#define PERFORMHTTPREQUEST_H

#include <curl/curl.h>
#include <iostream>
#include <string>
#include <vector>

class HTTPRequest {
private:
  // Callback function to handle response data
  static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                              std::string *s);

public:
  // Perform an HTTP request
  std::string performHttpRequest(const std::string &url,
                                 const std::string &method,
                                 const std::string &data = "",
                                 const std::vector<std::string> &headers = {});
};

#endif // PERFORMHTTPREQUEST_H
