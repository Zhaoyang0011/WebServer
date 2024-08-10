#include "Timer.h"

using namespace zyweb;

void net::Timer::restart(Timestamp now) {
  if (_repeat) {
    _expiration = addTime(now, _interval);
  } else {
    _expiration = Timestamp::invalid();
  }
}
