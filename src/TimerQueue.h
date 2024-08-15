#ifndef TIMERQUEUE_H_
#define TIMERQUEUE_H_

#include <set>
#include "base/types.h"
#include "base/Timestamp.h"
#include "Channel.h"

namespace zyweb::net {

class EventLoop;
class Timer;
class TimerId;

class TimerQueue {
 public:
  explicit TimerQueue(EventLoop *loop);
  ~TimerQueue();

  ///
  /// Schedules the callback to be run at given time,
  /// repeats if @c interval > 0.0.
  ///
  /// Must be thread safe. Usually be called from other threads.
  TimerId addTimer(TimerCallback cb,
                   Timestamp when,
                   double interval);

  void cancel(TimerId timerId);
 private:
  // FIXME: use unique_ptr<Timer> instead of raw pointers.
  // This requires heterogeneous comparison lookup (N3465) from C++14
  // so that we can find an T* in a set<unique_ptr<T>>.
  typedef std::pair<Timestamp, Timer *> Entry;
  typedef std::set<Entry> TimerList;
  typedef std::pair<Timer *, int64_t> ActiveTimer;
  typedef std::set<ActiveTimer> ActiveTimerSet;

  void addTimerInLoop(Timer *timer);
  void cancelInLoop(TimerId timerId);
  // called when timerfd alarms
  void handleRead();
  // move out all expired timers
  std::vector<Entry> getExpired(Timestamp now);
  void reset(const std::vector<Entry> &expired, Timestamp now);

  bool insert(Timer *timer);

  EventLoop *_loop;
  const int _timerfd;
  Channel _timerfdChannel;
  // Timer list sorted by expiration
  TimerList _timers;
  // for cancel()
  ActiveTimerSet _activeTimers;
  bool _callingExpiredTimers; /* atomic */
  ActiveTimerSet _cancelingTimers;
};

}

#endif
