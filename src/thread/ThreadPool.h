#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <functional>
#include <deque>
#include <memory>
#include "base/NonCopyable.h"
#include "base/types.h"
#include "util/Mutex.h"
#include "util/Condition.h"
#include "Thread.h"

namespace zyweb {

class ThreadPool : NonCopyable {
 public:
  typedef std::function<void()> Task;

  explicit ThreadPool(string nameArg = string("ThreadPool"));
  ~ThreadPool();

  // Must be called before start().
  void setMaxQueueSize(int maxSize) { _maxQueueSize = maxSize; }
  void setThreadInitCallback(const Task &cb) { _threadInitCallback = cb; }

  void start(int numThreads);
  void stop();

  const string &name() const { return _name; }

  size_t queueSize() const;

  // Could block if maxQueueSize > 0
  // Call after stop() will return immediately.
  // There is no move-only version of std::function in C++ as of C++14.
  // So we don't need to overload a const& and an && versions
  // as we do in (Bounded)BlockingQueue.
  // https://stackoverflow.com/a/25408989
  void run(Task f);

 private:
  bool isFull() const;
  void runInThread();
  Task take();

  mutable MutexLock _mutex;
  Condition _notEmpty;
  Condition _notFull;
  string _name;
  Task _threadInitCallback;
  std::vector<std::unique_ptr<Thread>> _threads;
  std::deque<Task> _queue;
  size_t _maxQueueSize;
  bool _running;
};

}

#endif
