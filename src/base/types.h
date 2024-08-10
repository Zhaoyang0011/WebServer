#ifndef BASE_TYPES_H_
#define BASE_TYPES_H_

#include <string>
#include <functional>

namespace zyweb {

using std::string;

typedef std::function<void()> TimerCallback;

template<typename To, typename From>
inline To implicit_cast(From const &f) {
  return f;
}

}

#endif
