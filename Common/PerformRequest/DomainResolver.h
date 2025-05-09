
#ifndef DOMAIN_RESOLVER_H
#define DOMAIN_RESOLVER_H

#include <string>
#include <vector>

class DomainResolver {
public:
  std::vector<std::string> getIPv4Addresses(const std::string &domain);

private:
  std::string domain_;
};

#endif // DOMAIN_RESOLVER_H
