#include "DomainResolver.h"
#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>

std::vector<std::string>
DomainResolver::getIPv4Addresses(const std::string &domain) {
  std::cout << "iam here " << domain << std::endl;
  std::vector<std::string> ipAddresses;
  struct addrinfo hints{}, *res, *p;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_CANONNAME;

  int status = getaddrinfo(domain.c_str(), nullptr, &hints, &res);
  if (status != 0) {
    std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
    return ipAddresses;
  }

  char ipstr[INET_ADDRSTRLEN];
  for (p = res; p != nullptr; p = p->ai_next) {
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
    void *addr = &(ipv4->sin_addr);
    inet_ntop(AF_INET, addr, ipstr, sizeof(ipstr));
    ipAddresses.emplace_back(std::string(ipstr));
  }

  for (const auto &ip : ipAddresses) {
    std::cout << "iam here -- here is ips " << ip << std::endl;
  }
  freeaddrinfo(res);
  return ipAddresses;
}
