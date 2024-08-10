#ifndef POLLER_H_
#define POLLER_H_

#include <vector>
#include <map>
#include "EventLoop.h"

namespace zyweb::net {

class Channel;

class Poller {

 public:
  typedef std::vector<Channel *> ChannelList;

  Poller(EventLoop *loop);
  virtual ~Poller();

  /// Polls the I/O events.
  /// Must be called in the loop thread.
  virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;

  /// Changes the interested I/O events.
  /// Must be called in the loop thread.
  virtual void updateChannel(Channel *channel) = 0;

  /// Remove the channel, when it destructs.
  /// Must be called in the loop thread.
  virtual void removeChannel(Channel *channel) = 0;

  virtual bool hasChannel(Channel *channel) const;

  static Poller *newDefaultPoller(EventLoop *loop);

  void assertInLoopThread() const {
    _ownerLoop->assertInLoopThread();
  }

 protected:
  typedef std::map<int, Channel *> ChannelMap;
  ChannelMap _channels;
 private:
  EventLoop *_ownerLoop;
};

}

#endif
