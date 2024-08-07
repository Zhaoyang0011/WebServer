//
// Created by szy on 8/7/24.
//

#ifndef _CURRENTTHREAD_H_
#define _CURRENTTHREAD_H_

#include <csignal>
#include <sys/syscall.h>
#include "base/types.h"

namespace zyweb {

namespace CurrentThread {

// internal
extern __thread int t_cachedTid;
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
extern __thread const char *t_threadName;

void cacheTid() {
  if (t_cachedTid == 0) {
    t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
    t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
  }
}

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

}  // namespace CurrentThread

}

#endif
