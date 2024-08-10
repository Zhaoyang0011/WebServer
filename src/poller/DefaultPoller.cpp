#include "poller/Poller.h"
#include "Channel.h"

using namespace zyweb;
using namespace net;

Poller *Poller::newDefaultPoller(EventLoop *loop) {
  return nullptr;
}