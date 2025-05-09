#include <algorithm>
#include <cctype>
#include <regex>
#include <string>
#include <vector>

struct DomainInfo {
  std::string base_url;
  std::string primary_domain;
};

std::string toLower(const std::string &input) {
  std::string result = input;
  std::transform(result.begin(), result.end(), result.begin(), ::tolower);
  return result;
}

std::string stripWWW(const std::string &domain) {
  if (domain.substr(0, 4) == "www.") {
    return domain.substr(4);
  }
  return domain;
}

std::string extractPrimaryDomain(const std::string &domain_raw) {
  std::string domain = toLower(stripWWW(domain_raw));

  size_t port_pos = domain.find(':');
  if (port_pos != std::string::npos) {
    domain = domain.substr(0, port_pos);
  }

  if (!domain.empty() && domain.front() == '[' && domain.back() == ']') {
    return ""; // IPv6
  }
  std::regex ip_regex(R"(^(\d{1,3}\.){3}\d{1,3}$)");
  if (std::regex_match(domain, ip_regex)) {
    return ""; // IPv4
  }
  std::vector<std::string> parts;
  size_t start = 0, end;
  while ((end = domain.find('.', start)) != std::string::npos) {
    parts.push_back(domain.substr(start, end - start));
    start = end + 1;
  }
  parts.push_back(domain.substr(start));

  if (parts.size() < 2) {
    return ""; // Not a valid domain
  }

  const std::vector<std::string> multi_tlds = {"co.uk",  "com.br", "org.uk",
                                               "ac.jp",  "gov.uk", "co.jp",
                                               "net.cn", "com.cn"};

  std::string last_two = parts[parts.size() - 2] + "." + parts.back();

  if (std::find(multi_tlds.begin(), multi_tlds.end(), last_two) !=
      multi_tlds.end()) {
    if (parts.size() >= 3) {
      return parts[parts.size() - 3] + "." + last_two;
    } else {
      return last_two; // No subdomain
    }
  }

  return last_two;
}

DomainInfo extractDomainInfo(const std::string &url) {
  DomainInfo info;
  std::smatch match;

  std::regex url_regex(R"(^(?:[a-z]+:\/\/)?(?:[^@\/\n]+@)?\[?([^\/:\]]+)\]?)",
                       std::regex::icase);

  if (std::regex_search(url, match, url_regex)) {
    info.base_url = match[1].str();
    info.primary_domain = extractPrimaryDomain(info.base_url);

    if (info.primary_domain == info.base_url || info.primary_domain.empty()) {
      info.primary_domain.clear();
    }
  }

  return info;
}
