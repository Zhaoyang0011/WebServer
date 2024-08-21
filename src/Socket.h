#ifndef SOCKET_H_
#define SOCKET_H_

#include "base/NonCopyable.h"
#include "InternetAddress.h"

namespace zyweb::net {

class Socket : NonCopyable {
 public:
  explicit Socket(int sockfd)
      : _sockfd(sockfd) {}

  // Socket(Socket&&) // move constructor in C++11
  ~Socket();

  int fd() const { return _sockfd; }
  // return true if success.
  bool getTcpInfo(struct tcp_info *) const;
  bool getTcpInfoString(char *buf, int len) const;

  /// abort if address in use
  void bindAddress(const InternetAddress &localaddr);
  /// abort if address in use
  void listen();

  /// On success, returns a non-negative integer that is
  /// a descriptor for the accepted socket, which has been
  /// set to non-blocking and close-on-exec. *peeraddr is assigned.
  /// On error, -1 is returned, and *peeraddr is untouched.
  int accept(InternetAddress *peeraddr);

  void shutdownWrite();

  ///
  /// Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
  ///
  void setTcpNoDelay(bool on);

  ///
  /// Enable/disable SO_REUSEADDR
  ///
  void setReuseAddr(bool on);

  ///
  /// Enable/disable SO_REUSEPORT
  ///
  void setReusePort(bool on);

  ///
  /// Enable/disable SO_KEEPALIVE
  ///
  void setKeepAlive(bool on);

 private:
  const int _sockfd;
};

}

#endif
