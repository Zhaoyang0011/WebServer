//
// Created by szy on 8/7/24.
//

#ifndef UTIL_COUNTDOWNLATCH_H_
#define UTIL_COUNTDOWNLATCH_H_

#include "util/Mutex.h"
#include "util/Condition.h"

namespace zyweb {

class CountDownLatch {
 public:
  explicit CountDownLatch(int count) : _mutex(), _condition(_mutex), _count(count) {}

  void wait();

  void countDown();

  int getCount() const;

 private:
  mutable MutexLock _mutex;
  Condition _condition;
  int _count;
};

}

#endif
