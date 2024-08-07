//
// Created by szy on 7/19/24.
//

#ifndef _THREAD_H
#define _THREAD_H

#include <atomic>
#include <functional>
#include <utility>
#include <sys/prctl.h>
#include "thread/CurrentThread.h"
#include "util/Condition.h"
#include "base/types.h"
#include "base/NonCopyable.h"
#include "util/CountDownLatch.h"
#include "base/Exception.h"

namespace zyweb {

typedef std::function<void()> ThreadFunc;

class Thread : NonCopyable {
 public:
  explicit Thread(ThreadFunc, string name = string());
  ~Thread();
  void start();
  int join(); // return pthread_join()

  bool started() const { return _started; }
  pthread_t pthreadId() const { return _pthreadId; }
  pid_t tid() const { return _tid; }
  const string &name() const { return _name; }
  static int numCreated() { return _numCreated; }

 private:
  void setDefaultName();
  bool _started;
  bool _joined;
  pthread_t _pthreadId;
  pid_t _tid;
  ThreadFunc _func;
  string _name;
  CountDownLatch _latch;
  static std::atomic<int> _numCreated;
};

struct ThreadData {
  ThreadFunc _func;
  string _name;
  pid_t *_tid;
  CountDownLatch *_latch;

  ThreadData(ThreadFunc func,
             string name,
             pid_t *tid,
             CountDownLatch *latch)
      : _func(std::move(func)),
        _name(std::move(name)),
        _tid(tid),
        _latch(latch) {}

  void runInThread() {
    *_tid = CurrentThread::tid();
    _tid = nullptr;
    _latch->countDown();
    _latch = nullptr;

    CurrentThread::t_threadName = _name.empty() ? "zywebThread" : _name.c_str();
    ::prctl(PR_SET_NAME, CurrentThread::t_threadName);
    try {
      _func();
      CurrentThread::t_threadName = "finished";
    }
    catch (const Exception &ex) {
      CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "exception caught in Thread %s\n", _name.c_str());
      fprintf(stderr, "reason: %s\n", ex.what());
      fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
      abort();
    }
    catch (const std::exception &ex) {
      CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "exception caught in Thread %s\n", _name.c_str());
      fprintf(stderr, "reason: %s\n", ex.what());
      abort();
    }
    catch (...) {
      CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "unknown exception caught in Thread %s\n", _name.c_str());
      throw; // rethrow
    }
  }
};

}

#endif
