#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <functional>
#include "base/NonCopyable.h"

namespace zyweb {

class ThreadPool : NonCopyable {
 public:
  typedef std::function<void()> Task;

};

}

#endif
