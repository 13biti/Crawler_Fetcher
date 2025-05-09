
#ifndef DOMAIN_RESOLVER_H
#define DOMAIN_RESOLVER_H

#include <string>
#include <vector>

class DomainResolver {
public:
  explicit DomainResolver(const std::string &domain);
  std::vector<std::string> getIPv4Addresses() const;

private:
  std::string domain_;
};

#endif // DOMAIN_RESOLVER_H
