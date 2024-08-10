#include "Poller.h"
#include "Channel.h"

using namespace zyweb;
using namespace net;

Poller::Poller(EventLoop *loop) : _ownerLoop(loop) {
}

Poller::~Poller() = default;

bool Poller::hasChannel(Channel *channel) const {
  assertInLoopThread();
  auto it = _channels.find(channel->fd());
  return it != _channels.end() && it->second == channel;
}
