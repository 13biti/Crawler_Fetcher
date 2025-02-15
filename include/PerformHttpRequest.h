#ifndef PERFORMHTTPREQUEST_H
#define PERFORMHTTPREQUEST_H

#include <string>
#include <vector>

class HTTPRequest {
public:
  struct HttpResponse {
    std::string data;
    long statusCode;
  };

  HttpResponse performHttpRequest(const std::string &url,
                                  const std::string &method,
                                  const std::string &data = "",
                                  const std::vector<std::string> &headers = {});

private:
  static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                              std::string *s);
};

#endif // PERFORMHTTPREQUEST_H
