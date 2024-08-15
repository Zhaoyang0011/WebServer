#include "poller/Poller.h"
#include "Channel.h"
#include "EpollPoller.h"

using namespace zyweb;
using namespace net;

Poller *Poller::newDefaultPoller(EventLoop *loop) {
  return new EpollPoller(loop);
}