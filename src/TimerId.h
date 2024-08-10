#ifndef TIMERID_H_
#define TIMERID_H_

#include <cstdint>

namespace zyweb::net {

class Timer;

class TimerId {
 public:
  TimerId()
      : _timer(nullptr),
        _sequence(0) {
  }

  TimerId(Timer *timer, int64_t seq)
      : _timer(timer),
        _sequence(seq) {
  }

  // default copy-constructor, dtor and assignment are okay
  friend class TimerQueue;

 private:
  Timer *_timer;
  int64_t _sequence;
};

}

#endif
