#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "EventLoop.h"

using namespace zyweb;
using namespace zyweb::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, const string nameArg)
    : _baseLoop(baseLoop),
      _name(nameArg),
      _started(false),
      _numThreads(0),
      _next(0) {}

EventLoopThreadPool::~EventLoopThreadPool() = default;

void EventLoopThreadPool::start(const EventLoopThreadPool::ThreadInitCallback &cb) {
  assert(!_started);
  _baseLoop->assertInLoopThread();

  _started = true;

  for (int i = 0; i < _numThreads; ++i) {
    char buf[_name.size() + 32];
    snprintf(buf, sizeof buf, "%s%d", _name.c_str(), i);
    auto *t = new EventLoopThread(cb, buf);
    _threads.push_back(std::unique_ptr<EventLoopThread>(t));
    _loops.push_back(t->startLoop());
  }
  if (_numThreads == 0 && cb) {
    cb(_baseLoop);
  }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
  _baseLoop->assertInLoopThread();
  assert(_started);
  EventLoop *loop = _baseLoop;

  if (!_loops.empty()) {
    // round-robin
    loop = _loops[_next];
    ++_next;
    if (implicit_cast<size_t>(_next) >= _loops.size()) {
      _next = 0;
    }
  }
  return loop;
}

EventLoop *EventLoopThreadPool::getLoopForHash(size_t hashCode) {
  _baseLoop->assertInLoopThread();
  EventLoop *loop = _baseLoop;
  if (!_loops.empty()) {
    loop = _loops[hashCode % _loops.size()];
  }
  return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops() {
  _baseLoop->assertInLoopThread();
  assert(_started);
  if (_loops.empty()) {
    return std::vector<EventLoop *>(1, _baseLoop);
  }
  return _loops;
}