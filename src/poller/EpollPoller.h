#ifndef POLLER_EPOLLPOLLER_H_
#define POLLER_EPOLLPOLLER_H_

#include "Poller.h"

namespace zyweb::net {

class EpollPoller : public Poller {
 public:
  explicit EpollPoller(EventLoop *loop);
  ~EpollPoller() override;

  Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;
  void updateChannel(Channel *channel) override;
  void removeChannel(Channel *channel) override;

 private:
  static const int kInitEventListSize = 16;

  static const char *operationToString(int op);

  void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
  void update(int operation, Channel *channel) const;

  typedef std::vector<struct epoll_event> EventList;

  int _epollfd;
  EventList _events;
};

}

#endif //WEBSERVER_SRC_POLLER_EPOLLPOLLER_H_
