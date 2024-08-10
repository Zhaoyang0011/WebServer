#include <cassert>
#include <sys/poll.h>
#include <sstream>
#include "Channel.h"
#include "base/types.h"
#include "EventLoop.h"

using namespace zyweb;
using namespace zyweb::net;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fd) :
    _loop(loop),
    _fd(fd),
    _events(0),
    _revents(0),
    _index(-1),
    _logHup(true),
    _tied(false),
    _eventHandling(false),
    _addedToLoop(false) {

}

Channel::~Channel() {
  assert(!_eventHandling);
  assert(!_addedToLoop);
  if (_loop->isInLoopThread()) {
    assert(!_loop->hasChannel(this));
  }
}

void Channel::handleEvent(Timestamp receiveTime) {
  std::shared_ptr<void> guard;
  if (_tied) {
    guard = _tie.lock();
    if (guard) {
      handleEventWithGuard(receiveTime);
    }
  } else {
    handleEventWithGuard(receiveTime);
  }
}

void Channel::tie(const std::shared_ptr<void> &obj) {
  _tie = obj;
  _tied = true;
}

string Channel::eventsToString(int fd, int ev) {
  std::ostringstream oss;
  oss << fd << ": ";
  if (ev & POLLIN)
    oss << "IN ";
  if (ev & POLLPRI)
    oss << "PRI ";
  if (ev & POLLOUT)
    oss << "OUT ";
  if (ev & POLLHUP)
    oss << "HUP ";
  if (ev & POLLRDHUP)
    oss << "RDHUP ";
  if (ev & POLLERR)
    oss << "ERR ";
  if (ev & POLLNVAL)
    oss << "NVAL ";
  return oss.str();
}

void Channel::update() {
  _addedToLoop = true;
  _loop->updateChannel(this);
}

void Channel::handleEventWithGuard(Timestamp receiveTime) {
  _eventHandling = true;

  if ((_revents & POLLHUP) && !(_revents & POLLIN) && _closeCallback) {
    _closeCallback();
  }

  if ((_revents & (POLLERR | POLLNVAL)) && _errorCallback) {
    _errorCallback();
  }

  if ((_revents & (POLLIN | POLLPRI | POLLRDHUP)) && _readCallback) {
    _readCallback(receiveTime);
  }

  if ((_revents & POLLOUT) && _writeCallback) {
    _writeCallback();
  }

  _eventHandling = false;
}

void Channel::remove() {
  assert(isNoneEvent());
  _addedToLoop = false;
  _loop->removeChannel(this);
}

string Channel::reventsToString() const {
  return eventsToString(_fd, _revents);
}

string Channel::eventsToString() const {
  return eventsToString(_fd, _events);
}
