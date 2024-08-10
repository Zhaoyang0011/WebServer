#ifndef _LOG_ASYNCLOGGER_H
#define _LOG_ASYNCLOGGER_H

#include <cstdio>
#include <string>
#include <memory>
#include "util/Mutex.h"
#include "util/CountDownLatch.h"
#include "thread/Thread.h"
#include "FixedBuffer.h"

namespace zyweb {

class AsyncLogger {

 public:

  AsyncLogger(const string &basename,
              off_t rollSize,
              int flushInterval = 3);

  ~AsyncLogger() {
    if (_running) {
      stop();
    }
  }

  void append(const char *logline, int len);

  void start() {
    _running = true;
    _thread.start();
    _latch.wait();
  }

  void stop() {
    _running = false;
    _cond.notify();
    _thread.join();
  }

 private:

  void threadFunc();

  typedef LargeBuffer Buffer;
  typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
  typedef BufferVector::value_type BufferPtr;

  const int _flushInterval;
  std::atomic<bool> _running;
  const string _basename;
  const off_t _rollsize;
  Thread _thread;
  CountDownLatch _latch;
  MutexLock _mutex;
  Condition _cond;
  BufferPtr _currentBuffer;
  BufferPtr _nextBuffer;
  BufferVector _buffers;
};

}

#endif
