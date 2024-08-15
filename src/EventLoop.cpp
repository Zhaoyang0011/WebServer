#include <cstring>
#include <sys/eventfd.h>
#include "EventLoop.h"
#include "poller/Poller.h"
#include "Channel.h"
#include "TimerQueue.h"

using namespace zyweb;
using namespace zyweb::net;

__thread EventLoop *t_loopInThisThread = nullptr;

const int kPollTimeMs = 10000;

int createEventfd() {
  int event_fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (event_fd < 0) {
    abort();
  }
  return event_fd;
}

EventLoop::EventLoop() :
    _looping(false),
    _quit(false),
    _eventHandling(false),
    _callingPendingFunctors(false),
    _iteration(0),
    _threadId(CurrentThread::tid()),
    _pollReturnTime(),
    _poller(Poller::newDefaultPoller(this)),
    _timerQueue(new TimerQueue(this)),
    _wakeupFd(createEventfd()),
    _wakeupChannel(new Channel(this, _wakeupFd)),
    _currentActiveChannel(nullptr) {
  // TODO: log error when there's another loop in thread
  if (!t_loopInThisThread) {
    t_loopInThisThread = this;
  }
  _wakeupChannel->setReadCallback(
      std::bind(&EventLoop::handleRead, this));
  // we are always reading the wakeupfd
  _wakeupChannel->enableReading();
}

EventLoop::~EventLoop() {
  _wakeupChannel->disableAll();
  _wakeupChannel->remove();
  ::close(_wakeupFd);
  t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
  assert(!_looping);
  assertInLoopThread();
  _looping = true;
  _quit = false;

  while (!_quit) {
    _activeChannels.clear();
    _pollReturnTime = _poller->poll(kPollTimeMs, &_activeChannels);
    ++_iteration;
//    if (Logger::logLevel() <= Logger::TRACE) {
//      printActiveChannels();
//    }
    _eventHandling = true;
    for (Channel *channel : _activeChannels) {
      _currentActiveChannel = channel;
      _currentActiveChannel->handleEvent(_pollReturnTime);
    }
    _currentActiveChannel = nullptr;
    _eventHandling = false;
    doPendingFunctors();
  }

//  LOG_TRACE << "EventLoop " << this << " stop looping";
  _looping = false;
}

void EventLoop::quit() {
  _quit = true;
  // There is a chance that loop() just executes while(!quit_) and exits,
  // then EventLoop destructs, then we are accessing an invalid object.
  // Can be fixed using mutex_ in both places.
  if (!isInLoopThread()) {
    wakeup();
  }
}

void EventLoop::runInLoop(EventLoop::Functor cb) {
  if (isInLoopThread()) {
    cb();
  } else {
    queueInLoop(std::move(cb));
  }
}

void EventLoop::queueInLoop(EventLoop::Functor cb) {
  {
    MutexLockGuard lock(_mutex);
    _pendingFunctors.push_back(std::move(cb));
  }

  if (!isInLoopThread() || _callingPendingFunctors) {
    wakeup();
  }
}

size_t EventLoop::queueSize() const {
  MutexLockGuard lock(_mutex);
  return _pendingFunctors.size();
}

TimerId EventLoop::runAt(Timestamp time, TimerCallback cb) {
  return _timerQueue->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay, TimerCallback cb) {
  return TimerId();
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb) {
  return TimerId();
}

void EventLoop::cancel(TimerId timerId) {

}

void EventLoop::wakeup() {
  uint64_t one = 1;
  ::write(_wakeupFd, &one, sizeof one);
}

void EventLoop::updateChannel(Channel *channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  _poller->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  if (_eventHandling) {
    assert(_currentActiveChannel == channel ||
        std::find(_activeChannels.begin(), _activeChannels.end(), channel) == _activeChannels.end());
  }
  _poller->removeChannel(channel);

}

bool EventLoop::hasChannel(Channel *channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  return _poller->hasChannel(channel);
}

EventLoop *EventLoop::getEventLoopOfCurrentThread() {
  return t_loopInThisThread;
}

void EventLoop::abortNotInLoopThread() {

}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ::read(_wakeupFd, &one, sizeof one);
}

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;
  _callingPendingFunctors = true;

  {
    MutexLockGuard lock(_mutex);
    functors.swap(_pendingFunctors);
  }

  for (const Functor &functor : functors) {
    functor();
  }
  _callingPendingFunctors = false;
}

void EventLoop::printActiveChannels() const {

}
