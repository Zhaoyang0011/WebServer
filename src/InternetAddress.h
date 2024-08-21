#ifndef INTERNETADDRESS_H_
#define INTERNETADDRESS_H_

#include <bits/sockaddr.h>
#include <netinet/in.h>
#include "base/types.h"

namespace zyweb::net {

class InternetAddress {
 public:
  /// Constructs an endpoint with given port number.
  /// Mostly used in TcpServer listening.
  explicit InternetAddress(uint16_t port = 0, bool loopbackOnly = false, bool ipv6 = false);

  /// Constructs an endpoint with given ip and port.
  /// @c ip should be "1.2.3.4"
  InternetAddress(string ip, uint16_t port, bool ipv6 = false);

  /// Constructs an endpoint with given struct @c sockaddr_in
  /// Mostly used when accepting new connections
  explicit InternetAddress(const struct sockaddr_in &addr)
      : _addr(addr) {}

  explicit InternetAddress(const struct sockaddr_in6 &addr)
      : _addr6(addr) {}

  sa_family_t family() const { return _addr.sin_family; }
  string toIp() const;
  string toIpPort() const;
  uint16_t port() const;

  // default copy/assignment are Okay

  const struct sockaddr *getSockAddr() const { return sockets::sockaddr_cast(&_addr6); }
  void setSockAddrInet6(const struct sockaddr_in6 &addr6) { _addr6 = addr6; }

  uint32_t ipv4NetEndian() const;
  uint16_t portNetEndian() const { return _addr.sin_port; }

  // resolve hostname to IP address, not changing port or sin_family
  // return true on success.
  // thread safe
  static bool resolve(string hostname, InternetAddress *result);
  // static std::vector<InetAddress> resolveAll(const char* hostname, uint16_t port = 0);

  // set IPv6 ScopeID
  void setScopeId(uint32_t scope_id);

 private:
  union {
    struct sockaddr_in _addr;
    struct sockaddr_in6 _addr6;
  };

};

}

#endif
