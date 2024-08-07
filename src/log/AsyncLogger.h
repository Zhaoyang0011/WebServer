#ifndef _LOG_ASYNCLOGGER_H
#define _LOG_ASYNCLOGGER_H

#include <cstdio>
#include <string>
#include "util/Mutex.h"

namespace zyweb {

class AsyncLogger {
 public:

 private:
  MutexLock lock;
};

}

#endif
