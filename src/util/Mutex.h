//
// Created by szy on 7/19/24.
//

#ifndef WEBSERVER_SRC_UTIL_MUTEX_H_
#define WEBSERVER_SRC_UTIL_MUTEX_H_

#include <pthread.h>
#include "thread/CurrentThread.h"
#include "base/NonCopyable.h"

namespace zyweb {

class MutexLock {
 public:
  MutexLock() : _threadId(0) {
    pthread_mutex_init(&_mutexLock, nullptr);
  }
  pthread_mutex_t *getPthreadMutex() {
    return &_mutexLock;
  }
  void lock() {
    pthread_mutex_lock(&_mutexLock);
  }
  void unlock() {
    pthread_mutex_unlock(&_mutexLock);
  }
  void unassignHolder() {
    _threadId = 0;
  }
  void assignHolder() {
    _threadId = CurrentThread::tid();
  }
 private:
  friend class Condition;
  class UnassignGuard : NonCopyable {
   public:
    explicit UnassignGuard(MutexLock &owner)
        : _owner(owner) {
      _owner.unassignHolder();
    }

    ~UnassignGuard() {
      _owner.assignHolder();
    }

   private:
    MutexLock &_owner;
  };
  pthread_mutex_t _mutexLock;
  pid_t _threadId;
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

#endif
