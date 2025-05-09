#include "../include/ResolverHelper.h"

using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_document;
std::string ResolverHelper::toLower(const std::string &input) {
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
std::string
ResolverHelper::extractPrimaryDomain(const std::string &domain_raw) {
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
DomainInfo ResolverHelper::extractDomainInfo(const std::string &url) {
  DomainResolver domainResolver;
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
    info.ips = domainResolver.getIPv4Addresses(info.base_url);
  }
  return info;
}

// Method 1: Create new domain group document
bool ResolverHelper::createNewDomainGroup(mongocxx::database &db,
                                          const DomainInfo &info) {
  try {
    auto collection = db["domain_groups"];

    document group_doc;
    group_doc << "primary_domain" << info.primary_domain << "ips" <<
        [&info](bsoncxx::builder::stream::array_context<> arr) {
          for (const auto &ip : info.ips) {
            arr << ip;
          }
        } << "domains"
              << bsoncxx::builder::stream::open_array << info.base_url
              << bsoncxx::builder::stream::close_array << "last_updated"
              << bsoncxx::types::b_date{std::chrono::system_clock::now()}
              << "request_count" << 0;

    auto result = collection.insert_one(group_doc.view());
    return result && result->inserted_id().type() == bsoncxx::type::k_oid;
  } catch (const std::exception &e) {
    std::cerr << "Error creating domain group: " << e.what() << std::endl;
    return false;
  }
}

bsoncxx::stdx::optional<bsoncxx::document::value>
ResolverHelper::findMatchingDomainGroup(mongocxx::database &db,
                                        const DomainInfo &info) {

  try {
    auto collection = db["domain_groups"];

    for (const auto &ip : info.ips) {
      auto filter = document{} << "ips" << ip << finalize;
      auto result = collection.find_one(filter.view());
      if (result) {
        return result;
      }
    }

    if (!info.primary_domain.empty()) {
      auto filter = document{} << "primary_domain" << info.primary_domain
                               << finalize;
      return collection.find_one(filter.view());
    }

    return bsoncxx::stdx::nullopt;
  } catch (const std::exception &e) {
    std::cerr << "Error finding domain group: " << e.what() << std::endl;
    return bsoncxx::stdx::nullopt;
  }
}

bool ResolverHelper::updateDomainGroup(mongocxx::database &db,
                                       const bsoncxx::document::view &group_doc,
                                       const DomainInfo &info) {
  try {
    auto collection = db["domain_groups"];

    auto domains_array = group_doc["domains"].get_array().value;
    for (const auto &domain : domains_array) {
      if (domain.get_string().value == info.base_url) {
        return true; // Already exists
      }
    }

    auto filter = document{} << "_id" << group_doc["_id"].get_oid().value
                             << finalize;

    document update;
    update << "$addToSet" << open_document << "domains" << info.base_url
           << "ips" <<
        [&info](bsoncxx::builder::stream::array_context<> arr) {
          for (const auto &ip : info.ips) {
            arr << ip;
          }
        } << close_document
           << "$set" << open_document << "last_updated"
           << bsoncxx::types::b_date{std::chrono::system_clock::now()}
           << close_document;

    auto result = collection.update_one(filter.view(), update.view());
    return result && result->modified_count() > 0;
  } catch (const std::exception &e) {
    std::cerr << "Error updating domain group: " << e.what() << std::endl;
    return false;
  }
}
std::pair<bool, std::string>
ResolverHelper::processDomain(mongocxx::database &db, std::string url) {
  auto info = extractDomainInfo(url);
  auto existing_group = findMatchingDomainGroup(db, info);
  if (existing_group) {
    bool success = updateDomainGroup(db, existing_group->view(), info);
    std::string group_id =
        !info.primary_domain.empty() ? info.primary_domain : info.base_url;
    return {success, group_id};
  } else {
    std::string group_id =
        !info.primary_domain.empty() ? info.primary_domain : info.base_url;
    return {createNewDomainGroup(db, info), group_id};
  }
}
std::string ResolverHelper::extractBaseUrl(const std::string &url) {
  std::smatch match;
  // Matches:
  // 1. Protocol (optional)
  // 2. Any auth (user@) (optional)
  // 3. The domain part (supports IPs and regular domains)
  std::regex url_regex(R"(^(?:[a-z]+:\/\/)?(?:[^@\/\n]+@)?([^\/:\?]+))",
                       std::regex::icase);

  if (std::regex_search(url, match, url_regex)) {
    std::string base = match[1].str();

    // Remove port number if present
    size_t port_pos = base.find(':');
    if (port_pos != std::string::npos) {
      base = base.substr(0, port_pos);
    }

    return base;
  }
  return "";
}
