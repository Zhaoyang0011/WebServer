//
// Created by szy on 7/19/24.
//

#ifndef _THREAD_H
#define _THREAD_H

#include <atomic>
#include <functional>
#include "base/types.h"
#include "base/NonCopyable.h"

namespace zyweb {

class Thread : NonCopyable {
 public:
  typedef std::function<void()> ThreadFunc;

  explicit Thread(ThreadFunc, const string &name = string());
  ~Thread();
  void start();
  int join(); // return pthread_join()

  bool started() const { return _started; }
  // pthread_t pthreadId() const { return pthreadId_; }
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

  static std::atomic<int> _numCreated;
};

}

#endif
