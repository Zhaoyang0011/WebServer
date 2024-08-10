#include <cassert>
#include <cerrno>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include "EpollPoller.h"
#include "Channel.h"

using namespace zyweb;
using namespace zyweb::net;

const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same.
static_assert(EPOLLIN == POLLIN, "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI, "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT, "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP, "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR, "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP, "epoll uses same flag values as poll");

EpollPoller::EpollPoller(EventLoop *loop) :
    Poller(loop),
    _epollfd(::epoll_create1(EPOLL_CLOEXEC)),
    _events(kInitEventListSize) {
}

EpollPoller::~EpollPoller() {
  ::close(_epollfd);
}

Timestamp EpollPoller::poll(int timeoutMs, Poller::ChannelList *activeChannels) {
  return Timestamp();
}

void EpollPoller::updateChannel(Channel *channel) {
  Poller::assertInLoopThread();
  const int index = channel->index();
  if (index == kNew || index == kDeleted) {
    // a new one, add with EPOLL_CTL_ADD
    int fd = channel->fd();
    if (index == kNew) {
      assert(_channels.find(fd) == _channels.end());
      _channels[fd] = channel;
    } else // index == kDeleted
    {
      assert(_channels.find(fd) != _channels.end());
      assert(_channels[fd] == channel);
    }
    channel->set_index(kAdded);
    update(EPOLL_CTL_ADD, channel);
  } else {
    // update existing one with EPOLL_CTL_MOD/DEL
    int fd = channel->fd();
    (void)fd;
    assert(_channels.find(fd) != _channels.end());
    assert(_channels[fd] == channel);
    assert(index == kAdded);
    if (channel->isNoneEvent()) {
      update(EPOLL_CTL_DEL, channel);
      channel->set_index(kDeleted);
    } else {
      update(EPOLL_CTL_MOD, channel);
    }
  }
}

void EpollPoller::removeChannel(Channel *channel) {
  Poller::assertInLoopThread();
  int fd = channel->fd();
}

const char *EpollPoller::operationToString(int op) {
  return nullptr;
}

void EpollPoller::fillActiveChannels(int numEvents, Poller::ChannelList *activeChannels) const {

}

void EpollPoller::update(int operation, Channel *channel) const {
  struct epoll_event event{};
  memset(&event, 0, sizeof event);
  event.events = channel->events();
  event.data.ptr = channel;
  int fd = channel->fd();
  // TODO: add err log.
  ::epoll_ctl(_epollfd, operation, fd, &event);
}
