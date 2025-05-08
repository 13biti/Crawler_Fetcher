#include "../include/LinkExtractor.h"
#include <algorithm>
#include <iostream>
#include <libxml2/libxml/HTMLparser.h>
#include <libxml2/libxml/uri.h>
#include <libxml2/libxml/xpath.h>
#include <regex>

std::vector<std::string>
LinkExtractor::ExtractRedirectLinks(const std::string &html_content,
                                    const std::string &base_url) {
  std::vector<std::string> links;
  // Parse HTML document
  htmlDocPtr doc = htmlReadMemory(html_content.c_str(), html_content.size(),
                                  base_url.c_str(), NULL,
                                  HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR |
                                      HTML_PARSE_NOWARNING | HTML_PARSE_NONET);
  if (!doc) {
    return links;
  }

  // Create XPath context - FIXED THIS LINE
  xmlXPathContextPtr context = xmlXPathNewContext(doc);
  if (!context) {
    xmlFreeDoc(doc);
    return links;
  }

  // Rest of the code remains the same...
  // Evaluate XPath expression for <a> tags with href
  xmlXPathObjectPtr result =
      xmlXPathEvalExpression((const xmlChar *)"//a/@href", context);
  if (!result) {
    xmlXPathFreeContext(context);
    xmlFreeDoc(doc);
    return links;
  }

  // Process found links
  xmlNodeSetPtr nodes = result->nodesetval;
  if (nodes && !xmlXPathNodeSetIsEmpty(nodes)) {
    for (int i = 0; i < nodes->nodeNr; i++) {
      xmlNodePtr node = nodes->nodeTab[i];
      xmlChar *href = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
      if (href) {
        std::string link((char *)href);
        xmlFree(href);

        // Resolve relative URLs if base_url is provided
        if (!base_url.empty() && !link.empty()) {
          if (link.find("http://") != 0 && link.find("https://") != 0) {
            xmlChar *full_url = xmlBuildURI((xmlChar *)link.c_str(),
                                            (xmlChar *)base_url.c_str());
            if (full_url) {
              link = (char *)full_url;
              xmlFree(full_url);
            }
          }
        }

        // Add to results if it's a valid HTTP/HTTPS URL
        if (link.find("http://") == 0 || link.find("https://") == 0) {
          links.push_back(link);
        }
      }
    }
  }

  // Cleanup
  xmlXPathFreeObject(result);
  xmlXPathFreeContext(context);
  xmlFreeDoc(doc);

  // Remove duplicates
  std::sort(links.begin(), links.end());
  links.erase(std::unique(links.begin(), links.end()), links.end());

  return links;
}

std::string LinkExtractor::GetBaseUrl(const std::string &url) {
  if (url.empty())
    return "";

  size_t pos = url.find("://");
  if (pos == std::string::npos)
    return "";

  pos += 3; // Skip past ://
  pos = url.find('/', pos);
  if (pos == std::string::npos) {
    return url;
  }
  return url.substr(0, pos);
}
std::string LinkExtractor::GetBaseUrl(const std::string &url,
                                      const std::string &html_content) {
  // First check for <base href> in HTML
  std::regex base_regex(R"(<base\s+href=["']([^"']+)["'])");
  std::smatch matches;
  if (std::regex_search(html_content, matches, base_regex)) {
    return matches[1].str(); // Use <base href> if found
  }

  // Fall back to domain extraction
  return GetBaseUrl(url); // Your existing implementation
}
