//
// Created by szy on 8/10/24.
//

#ifndef TIMER_H_
#define TIMER_H_

#include <atomic>
#include "base/NonCopyable.h"
#include "base/Timestamp.h"

namespace zyweb::net {

class Timer : NonCopyable {
 public:
  Timer(TimerCallback cb, Timestamp when, double interval)
      : _callback(std::move(cb)),
        _expiration(when),
        _interval(interval),
        _repeat(interval > 0.0),
        _sequence(++s_numCreated) {}

  void run() const {
    _callback();
  }

  Timestamp expiration() const { return _expiration; }
  bool repeat() const { return _repeat; }
  int64_t sequence() const { return _sequence; }

  void restart(Timestamp now);

  static int64_t numCreated() { return s_numCreated; }

 private:
  const TimerCallback _callback;
  Timestamp _expiration;
  const double _interval;
  const bool _repeat;
  const int64_t _sequence;

  static std::atomic<int64_t> s_numCreated;

};

}

#endif //WEBSERVER_SRC_TIMER_H_
