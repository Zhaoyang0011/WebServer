//
// Created by szy on 7/19/24.
//

#ifndef WEBSERVER_SRC_UTIL_MUTEX_H_
#define WEBSERVER_SRC_UTIL_MUTEX_H_

#include <pthread.h>

namespace zyweb {

class MutexLock {
 public:
  MutexLock() : threadId(0) {
    pthread_mutex_init(&mutexLock, nullptr);
  }
  void lock() {
    pthread_mutex_lock(&mutexLock);
  }
  void unlock() {
    pthread_mutex_unlock(&mutexLock);
  }
 private:
  pthread_mutex_t mutexLock;
  pid_t threadId;
};

class MutexLockGuard {
 public:
  explicit MutexLockGuard(MutexLock &lock)
      : lock(lock) {
    lock.lock();
  }

  ~MutexLockGuard() {
    lock.unlock();
  }

 private:
  MutexLock &lock;
};

}

#endif //WEBSERVER_SRC_UTIL_MUTEX_H_
