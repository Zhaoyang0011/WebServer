//
// Created by szy on 8/7/24.
//

#ifndef THREAD_CURRENTTHREAD_H_
#define THREAD_CURRENTTHREAD_H_

#include <csignal>
#include <sys/syscall.h>
#include "base/types.h"

namespace zyweb::CurrentThread {

// internal
extern __thread int t_cachedTid;
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
extern __thread const char *t_threadName;

void cacheTid();

inline int tid() {
  if (__builtin_expect(t_cachedTid == 0, 0)) {
    cacheTid();
  }
  return t_cachedTid;
}

inline const char *tidString() // for logging
{
  return t_tidString;
}

inline int tidStringLength() // for logging
{
  return t_tidStringLength;
}

inline const char *name() {
  return t_threadName;
}

bool isMainThread();

void sleepUsec(int64_t usec);

string stackTrace(bool demangle);

} // namespace zyweb::CurrentThread



#endif
