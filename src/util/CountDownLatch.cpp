#include "CountDownLatch.h"

void zyweb::CountDownLatch::wait() {
  MutexLockGuard gard(_mutex);
  while (_count > 0) {
    _condition.wait();
  }
}

void zyweb::CountDownLatch::countDown() {
  MutexLockGuard gard(_mutex);
  if (--_count == 0) {
    _condition.notifyAll();
  }
}

int zyweb::CountDownLatch::getCount() const {
  return _count;
}
