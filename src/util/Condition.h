#ifndef UTIL_CONDITION_H_
#define UTIL_CONDITION_H_

#include "util/Mutex.h"
#include "base/NonCopyable.h"

namespace zyweb {

class Condition : NonCopyable {
 public:
  explicit Condition(MutexLock &lock) : _mutex(lock) {
    pthread_cond_init(&_pcond, nullptr);
  }

  ~Condition() {
    pthread_cond_destroy(&_pcond);
  }

  void wait() {
    MutexLock::UnassignGuard ug(_mutex);
    pthread_cond_wait(&_pcond, _mutex.getPthreadMutex());
  }

  void notify() {
    pthread_cond_signal(&_pcond);
  }

  void notifyAll() {
    pthread_cond_broadcast(&_pcond);
  }

 private:
  MutexLock &_mutex;
  pthread_cond_t _pcond{};
};

}

#endif
