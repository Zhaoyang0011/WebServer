#ifndef EVENTLOOPTHREAD_H_
#define EVENTLOOPTHREAD_H_

#include <functional>
#include <string>
#include "base/NonCopyable.h"
#include "base/types.h"
#include "thread/Thread.h"

namespace zyweb::net {

class EventLoop;

class EventLoopThread : NonCopyable {
 public:
  typedef std::function<void(EventLoop *)> ThreadInitCallback;

  explicit EventLoopThread(ThreadInitCallback cb = ThreadInitCallback(),
                           const string &name = string());
  ~EventLoopThread();
  EventLoop *startLoop();

 private:
  void threadFunc();

  EventLoop *_loop;
  bool _existing;
  Thread _thread;
  MutexLock _mutex;
  Condition _cond;
  ThreadInitCallback _callback;
};

}

#endif
