#ifndef EVENTLOOPTHREADPOOL_H_
#define EVENTLOOPTHREADPOOL_H_

#include <functional>
#include <string>
#include <memory>
#include "base/types.h"
#include "base/NonCopyable.h"

namespace zyweb::net {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : NonCopyable {
 public:
  typedef std::function<void(EventLoop *)> ThreadInitCallback;

  EventLoopThreadPool(EventLoop *baseLoop, string nameArg);
  ~EventLoopThreadPool();
  void setThreadNum(int numThreads) { _numThreads = numThreads; }
  void start(const ThreadInitCallback &cb = ThreadInitCallback());

  // valid after calling start()
  /// round-robin
  EventLoop *getNextLoop();

  /// with the same hash code, it will always return the same EventLoop
  EventLoop *getLoopForHash(size_t hashCode);

  std::vector<EventLoop *> getAllLoops();

  bool started() const { return _started; }

  const string &name() const { return _name; }

 private:

  EventLoop *_baseLoop;
  string _name;
  bool _started;
  int _numThreads;
  int _next;
  std::vector<std::unique_ptr<EventLoopThread>> _threads;
  std::vector<EventLoop *> _loops;

};

}

#endif
