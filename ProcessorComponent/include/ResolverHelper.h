#ifndef RESOLVERHELPER_H
#define RESOLVERHELPER_H

#include "../../Common/PerformRequest/DomainResolver.h"
#include "regex"
#include <bsoncxx/array/view.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/types/bson_value/view.hpp>
#include <iostream>
#include <mongocxx/collection.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mutex>
#include <ostream>
#include <set>
#include <string>
#include <vector>
struct DomainInfo {
  std::string base_url;
  std::string primary_domain;
  std::vector<std::string> ips;
};

class ResolverHelper {
public:
  std::pair<bool, std::string> processDomain(mongocxx::database &db,
                                             std::string url);

  std::string extractBaseUrl(const std::string &url);

private:
  bsoncxx::stdx::optional<bsoncxx::document::value>
  findMatchingDomainGroup(mongocxx::database &db, const DomainInfo &info);
  bool updateDomainGroup(mongocxx::database &db,
                         const bsoncxx::document::view &group_doc,
                         const DomainInfo &info);
  std::string toLower(const std::string &input);
  std::string stripWWW(const std::string &domain);
  std::string extractPrimaryDomain(const std::string &domain_raw);
  DomainInfo extractDomainInfo(const std::string &url);
  bool createNewDomainGroup(mongocxx::database &db, const DomainInfo &info);
};

#endif // RESOLVERHELPER_H
