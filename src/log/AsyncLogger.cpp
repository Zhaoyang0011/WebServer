#include "AsyncLogger.h"

#include <memory>

zyweb::AsyncLogger::AsyncLogger(const std::string &basename, off_t rollSize, int flushInterval) :
    _flushInterval(flushInterval),
    _rollsize(rollSize),
    _thread([this] { threadFunc(); }, basename),
    _latch(1),
    _cond(_mutex),
    _currentBuffer(new Buffer()),
    _nextBuffer(new Buffer()),
    _buffers() {
  _currentBuffer->bzero();
  _nextBuffer->bzero();
  _buffers.reserve(16);
}

void zyweb::AsyncLogger::threadFunc() {

  assert(_running == true);
  _latch.countDown();
}

void zyweb::AsyncLogger::append(const char *logline, int len) {
  zyweb::MutexLockGuard lock(_mutex);

  // simply write to current buffer
  if (_currentBuffer->avail() > len) {
    _currentBuffer->append(logline, len);
    return;
  }

  // push current buffer and switch buffer
  _buffers.push_back(std::move(_currentBuffer));
  if (_nextBuffer) {
    _currentBuffer = std::move(_nextBuffer);
  } else {
    _currentBuffer = std::make_unique<Buffer>(); // Rarely happens
  }
  _currentBuffer->append(logline, len);
  _cond.notify();
}
