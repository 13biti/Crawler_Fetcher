#ifndef LINK_EXTRACTOR_H
#define LINK_EXTRACTOR_H

#include <curl/curl.h>
#include <string>
#include <vector>

class LinkExtractor {
public:
  // Extract redirect links from HTML content
  static std::vector<std::string>
  ExtractRedirectLinks(const std::string &html_content,
                       const std::string &base_url = "");

  // Helper function to get base URL
  static std::string GetBaseUrl(const std::string &url);

private:
  // HTML parser callback
  static size_t HtmlParserCallback(void *contents, size_t size, size_t nmemb,
                                   void *userp);
};

#endif // LINK_EXTRACTOR_H
