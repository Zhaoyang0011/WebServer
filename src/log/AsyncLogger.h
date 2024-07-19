#ifndef _LOG_ASYNCLOGGER_H
#define _LOG_ASYNCLOGGER_H

#include <cstdio>
#include <string>

namespace zyweb {

class AsyncLogger {
 public:
  AsyncLogger(const std::string &basename,
               off_t rollSize,
               int flushInterval = 3);

  ~AsyncLogger() {
    if (running_) {
      stop();
    }
  }

  void append(const char *logline, int len);

  void start() {
    running_ = true;
    thread_.start();
    latch_.wait();
  }

  void stop()
  NO_THREAD_SAFETY_ANALYSIS
  {
    running_ = false;
    cond_.notify();
    thread_.join();
  }

 private:

  void threadFunc();

  typedef muduo::detail::FixedBuffer<muduo::detail::kLargeBuffer> Buffer;
  typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
  typedef BufferVector::value_type BufferPtr;

  const int flushInterval_;
  std::atomic<bool> running_;
  const string basename_;
  const off_t rollSize_;
  muduo::Thread thread_;
  muduo::CountDownLatch latch_;
  muduo::MutexLock mutex_;
  muduo::Condition cond_
  GUARDED_BY(mutex_);
  BufferPtr currentBuffer_
  GUARDED_BY(mutex_);
  BufferPtr nextBuffer_
  GUARDED_BY(mutex_);
  BufferVector buffers_
  GUARDED_BY(mutex_);
};

}

#endif
