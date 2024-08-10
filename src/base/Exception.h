//
// Created by szy on 8/7/24.
//

#ifndef BASE_EXCEPTION_H_
#define BASE_EXCEPTION_H_

#include <exception>
#include "base/types.h"
#include "thread/CurrentThread.h"

namespace zyweb {

class Exception : public std::exception {
 public:
  Exception(string msg)
      : _message(std::move(msg)),
        _stack(CurrentThread::stackTrace(/*demangle=*/false)) {};
  ~Exception() noexcept override = default;

  // default copy-ctor and operator= are okay.

  const char *what() const noexcept override {
    return _message.c_str();
  }

  const char *stackTrace() const noexcept {
    return _stack.c_str();
  }

 private:
  string _message;
  string _stack;
};

}
#endif //WEBSERVER_SRC_BASE_EXCEPTION_H_
