//
// Created by szy on 7/19/24.
//

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

#endif //WEBSERVER_SRC_THREAD_THREADPOOL_H_
