#include "../include/LinkExtractor.h"
#include <algorithm>
#include <cctype>
#include <regex>

LinkExtractor::LinkExtractor(const std::string &html_content)
    : html_content_(html_content) {}

std::vector<std::pair<std::string, std::string>>
LinkExtractor::extractAllLinks() const {
  std::vector<std::pair<std::string, std::string>> all_links;

  // Extract standard <a> links
  auto a_links = extractTagLinks("a", "href");
  all_links.insert(all_links.end(), a_links.begin(), a_links.end());

  // Extract <link> tags (often used for stylesheets, etc.)
  auto link_tags = extractTagLinks("link", "href");
  all_links.insert(all_links.end(), link_tags.begin(), link_tags.end());

  // Extract <script> tags
  auto script_links = extractTagLinks("script", "src");
  all_links.insert(all_links.end(), script_links.begin(), script_links.end());

  // Extract <img> tags
  auto img_links = extractTagLinks("img", "src");
  all_links.insert(all_links.end(), img_links.begin(), img_links.end());

  // Extract <iframe> tags
  auto iframe_links = extractTagLinks("iframe", "src");
  all_links.insert(all_links.end(), iframe_links.begin(), iframe_links.end());

  // Extract <form> action URLs
  auto form_actions = extractTagLinks("form", "action");
  all_links.insert(all_links.end(), form_actions.begin(), form_actions.end());

  // Remove duplicates
  std::sort(all_links.begin(), all_links.end());
  all_links.erase(std::unique(all_links.begin(), all_links.end()),
                  all_links.end());

  return all_links;
}

std::vector<std::pair<std::string, std::string>>
LinkExtractor::extractExternalLinks(const std::string &base_url) const {
  auto all_links = extractAllLinks();
  std::vector<std::pair<std::string, std::string>> external_links;

  std::copy_if(all_links.begin(), all_links.end(),
               std::back_inserter(external_links),
               [&base_url](const auto &link) {
                 return isExternalLink(link.second, base_url);
               });

  return external_links;
}

std::vector<std::pair<std::string, std::string>>
LinkExtractor::extractInternalLinks(const std::string &base_url) const {
  auto all_links = extractAllLinks();
  std::vector<std::pair<std::string, std::string>> internal_links;

  std::copy_if(all_links.begin(), all_links.end(),
               std::back_inserter(internal_links),
               [&base_url](const auto &link) {
                 return !isExternalLink(link.second, base_url);
               });

  return internal_links;
}

std::vector<std::pair<std::string, std::string>>
LinkExtractor::extractTagLinks(const std::string &tag_name,
                               const std::string &attribute) const {
  std::vector<std::pair<std::string, std::string>> links;
  std::regex pattern("<" + tag_name + "[^>]*" + attribute +
                         "=\"([^\"]*)\"[^>]*>(.*?)</" + tag_name + ">",
                     std::regex_constants::icase);

  auto words_begin =
      std::sregex_iterator(html_content_.begin(), html_content_.end(), pattern);
  auto words_end = std::sregex_iterator();

  for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
    std::smatch match = *i;
    if (match.size() >= 3) {
      std::string url = match[1].str();
      std::string text = match[2].str();

      // Clean up text (remove extra whitespace and tags)
      text = std::regex_replace(text, std::regex("<[^>]*>"), "");
      text = std::regex_replace(text, std::regex("\\s+"), " ");
      text.erase(std::remove_if(text.begin(), text.end(),
                                [](unsigned char c) {
                                  return std::isspace(c) && c != ' ';
                                }),
                 text.end());

      if (!url.empty()) {
        links.emplace_back(text, url);
      }
    }
  }

  return links;
}

bool LinkExtractor::isExternalLink(const std::string &link,
                                   const std::string &base_url) {
  if (link.empty())
    return false;

  // Check for common URL patterns
  if (link.find("http://") == 0 || link.find("https://") == 0) {
    std::string base_domain = getBaseUrl(base_url);
    std::string link_domain = getBaseUrl(link);

    return base_domain != link_domain;
  }

  // Relative links are internal
  return false;
}

std::string LinkExtractor::getBaseUrl(const std::string &url) {
  std::regex domain_regex(R"(^(https?://[^/]+))", std::regex_constants::icase);
  std::smatch match;

    if (std::regex_search(url, match, domain_regex) {
    return match[1].str();
    }
    
    return "";
}
