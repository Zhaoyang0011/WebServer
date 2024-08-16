#include "EventLoopThread.h"

#include <utility>
#include "EventLoop.h"

using namespace zyweb;
using namespace zyweb::net;

EventLoopThread::EventLoopThread(EventLoopThread::ThreadInitCallback cb,
                                 const string &name)
    : _loop(nullptr),
      _existing(false),
      _thread([this] { threadFunc(); }, name),
      _mutex(),
      _cond(_mutex),
      _callback(std::move(cb)) {}

EventLoopThread::~EventLoopThread() {
  _existing = true;
  if (_loop != nullptr) // not 100% race-free, eg. threadFunc could be running callback_.
  {
    // still a tiny chance to call destructed object, if threadFunc exits just now.
    // but when EventLoopThread destructs, usually programming is exiting anyway.
    _loop->quit();
    _thread.join();
  }
}

EventLoop *EventLoopThread::startLoop() {
  assert(!_thread.started());
  _thread.start();

  EventLoop *loop = nullptr;
  {
    MutexLockGuard lock(_mutex);
    while (_loop == nullptr) {
      _cond.wait();
    }
    loop = _loop;
  }

  return loop;
}

void EventLoopThread::threadFunc() {
  EventLoop loop;

  if (_callback) {
    _callback(&loop);
  }

  {
    MutexLockGuard lock(_mutex);
    _loop = &loop;
    _cond.notify();
  }

  loop.loop();
  //assert(exiting_);
  MutexLockGuard lock(_mutex);
  _loop = nullptr;
}
