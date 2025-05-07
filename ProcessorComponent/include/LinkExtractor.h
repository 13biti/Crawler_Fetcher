#ifndef LINK_EXTRACTOR_H
#define LINK_EXTRACTOR_H

#include <string>
#include <utility> // for std::pair
#include <vector>
// this is not my own code , i find it somewhere
class LinkExtractor {
public:
  explicit LinkExtractor(const std::string &html_content);

  std::vector<std::pair<std::string, std::string>> extractAllLinks() const;

  std::vector<std::pair<std::string, std::string>>
  extractExternalLinks(const std::string &base_url) const;

  std::vector<std::pair<std::string, std::string>>
  extractInternalLinks(const std::string &base_url) const;

private:
  std::string html_content_;

  std::vector<std::pair<std::string, std::string>>
  extractTagLinks(const std::string &tag_name,
                  const std::string &attribute) const;
  static bool isExternalLink(const std::string &link,
                             const std::string &base_url);
  static std::string getBaseUrl(const std::string &url);
};

#endif // LINK_EXTRACTOR_H
