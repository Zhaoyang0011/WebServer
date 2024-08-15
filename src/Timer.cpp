#include "Timer.h"

using namespace zyweb;
using namespace zyweb::net;

std::atomic<int64_t> Timer::s_numCreated;

void Timer::restart(Timestamp now) {
  if (_repeat) {
    _expiration = addTime(now, _interval);
  } else {
    _expiration = Timestamp::invalid();
  }
}
