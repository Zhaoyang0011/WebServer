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
//  LOG_TRACE << "fd total count " << channels_.size();
  int numEvents = ::epoll_wait(_epollfd,
                               &*_events.begin(),
                               static_cast<int>(_events.size()),
                               timeoutMs);
  int savedErrno = errno;
  if (numEvents > 0) {
//    LOG_TRACE << numEvents << " events happened";
    fillActiveChannels(numEvents, activeChannels);
    if (implicit_cast<size_t>(numEvents) == _events.size()) {
      _events.resize(_events.size() * 2);
    }
  } else if (numEvents == 0) {
//    LOG_TRACE << "nothing happened";
  } else {
    // error happens, log uncommon ones
    if (savedErrno != EINTR) {
      errno = savedErrno;
//      LOG_SYSERR << "EPollPoller::poll()";
    }
  }
  return Timestamp::now();
}

void EpollPoller::updateChannel(Channel *channel) {
  Poller::assertInLoopThread();
  const int index = channel->index();
  int fd = channel->fd();
  if (index == kNew) {
    assert(_channels.find(fd) == _channels.end());
    _channels[fd] = channel;
    channel->set_index(kAdded);
    update(EPOLL_CTL_ADD, channel);
  } else if (index == kDeleted) {
    assert(_channels.find(fd) != _channels.end());
    assert(_channels[fd] == channel);
    channel->set_index(kAdded);
    update(EPOLL_CTL_ADD, channel);
  } else {
    // update existing one with EPOLL_CTL_MOD/DEL
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
  assert(_channels.find(fd) != _channels.end());
  assert(_channels[fd] == channel);
  assert(channel->isNoneEvent());
  int index = channel->index();
  assert(index == kAdded || index == kDeleted);
  size_t n = _channels.erase(fd);
  (void)n;
  assert(n == 1);

  if (index == kAdded) {
    update(EPOLL_CTL_DEL, channel);
  }
  channel->set_index(kNew);
}

const char *EpollPoller::operationToString(int op) {
  switch (op) {
    case EPOLL_CTL_ADD:return "ADD";
    case EPOLL_CTL_DEL:return "DEL";
    case EPOLL_CTL_MOD:return "MOD";
    default:assert(false && "ERROR op");
  }
  return "Unknown Operation";
}

void EpollPoller::fillActiveChannels(int numEvents, Poller::ChannelList *activeChannels) const {
  assert(implicit_cast<size_t>(numEvents) <= _events.size());
  for (int i = 0; i < numEvents; ++i) {
    auto *channel = static_cast<Channel *>(_events[i].data.ptr);
#ifndef NDEBUG
    int fd = channel->fd();
    auto it = _channels.find(fd);
    assert(it != _channels.end());
    assert(it->second == channel);
#endif
    channel->set_revents(_events[i].events);
    activeChannels->push_back(channel);
  }
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
