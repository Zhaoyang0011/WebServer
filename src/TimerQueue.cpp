#include <sys/timerfd.h>
#include <unistd.h>
#include <cstring>
#include "EventLoop.h"
#include "TimerQueue.h"
#include "TimerId.h"
#include "Timer.h"

using namespace zyweb;
using namespace zyweb::net;

int createTimerfd() {
  return ::timerfd_create(CLOCK_MONOTONIC,
                          TFD_NONBLOCK | TFD_CLOEXEC);
}

struct timespec timeFromNow(Timestamp when) {
  int64_t microseconds = when.microSecondsSinceEpoch()
      - Timestamp::now().microSecondsSinceEpoch();
  if (microseconds < 100) {
    microseconds = 100;
  }
  struct timespec ts{};
  ts.tv_sec = static_cast<time_t>(
      microseconds / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(
      (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
  return ts;
}

void readTimerfd(int timerfd, Timestamp now) {
  uint64_t howmany;
  ::read(timerfd, &howmany, sizeof howmany);
}

void resetTimerfd(int timerfd, Timestamp expiration) {
  // wake up loop by timerfd_settime()
  struct itimerspec newValue{};
  struct itimerspec oldValue{};
  memset(&newValue, 0, sizeof newValue);
  memset(&oldValue, 0, sizeof oldValue);
  newValue.it_value = timeFromNow(expiration);
  ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
}

TimerQueue::TimerQueue(EventLoop *loop) :
    _loop(loop),
    _timerfd(createTimerfd()),
    _timerfdChannel(loop, _timerfd),
    _timers(),
    _activeTimers(),
    _callingExpiredTimers(false),
    _cancelingTimers() {
  _timerfdChannel.setReadCallback(
      std::bind(&TimerQueue::handleRead, this));
  // we are always reading the timerfd, we disarm it with timerfd_settime.
  _timerfdChannel.enableReading();
}

TimerQueue::~TimerQueue() {
  _timerfdChannel.disableAll();
  _timerfdChannel.remove();
  ::close(_timerfd);
  // do not remove channel, since we're in EventLoop::dtor();
  for (const Entry &timer : _timers) {
    delete timer.second;
  }

}

TimerId TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval) {
  auto *timer = new Timer(std::move(cb), when, interval);
  _loop->runInLoop(
      [this, timer] { addTimerInLoop(timer); });
  return {timer, timer->sequence()};
}

void TimerQueue::cancel(TimerId timerId) {
  _loop->runInLoop(
      [this, timerId] { cancelInLoop(timerId); });
}

void TimerQueue::addTimerInLoop(Timer *timer) {
  _loop->assertInLoopThread();
  bool earliestChanged = insert(timer);

  if (earliestChanged) {
    resetTimerfd(_timerfd, timer->expiration());
  }
}

void TimerQueue::cancelInLoop(TimerId timerId) {
  _loop->assertInLoopThread();
  assert(_timers.size() == _activeTimers.size());

  ActiveTimer timer(timerId._timer, timerId._sequence);
  auto it = _activeTimers.find(timer);

  if (it != _activeTimers.end()) {
    size_t n = _timers.erase(Entry(it->first->expiration(), it->first));
    assert(n == 1);
    (void)n;
    delete it->first; // FIXME: no delete please
    _activeTimers.erase(it);
  } else if (_callingExpiredTimers) {
    _cancelingTimers.insert(timer);
  }
  assert(_timers.size() == _activeTimers.size());
}

void TimerQueue::handleRead() {
  _loop->assertInLoopThread();
  Timestamp now(Timestamp::now());
  readTimerfd(_timerfd, now);

  std::vector<Entry> expired = getExpired(now);

  _callingExpiredTimers = true;
  _cancelingTimers.clear();
  // safe to callback outside critical section
  for (const Entry &it : expired) {
    it.second->run();
  }
  _callingExpiredTimers = false;

  reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
  assert(_timers.size() == _activeTimers.size());
  std::vector<Entry> expired;
  Entry sentry(now, reinterpret_cast<Timer *>(UINTPTR_MAX));
  auto end = _timers.lower_bound(sentry);
  assert(end == _timers.end() || now < end->first);
  std::copy(_timers.begin(), end, back_inserter(expired));
  _timers.erase(_timers.begin(), end);

  for (const Entry &it : expired) {
    ActiveTimer timer(it.second, it.second->sequence());
    size_t n = _activeTimers.erase(timer);
    assert(n == 1);
    (void)n;
  }

  assert(_timers.size() == _activeTimers.size());
  return expired;;
}

void TimerQueue::reset(const std::vector<Entry> &expired, Timestamp now) {
  Timestamp nextExpire;

  for (const Entry &it : expired) {
    ActiveTimer timer(it.second, it.second->sequence());
    if (it.second->repeat()
        && _cancelingTimers.find(timer) == _cancelingTimers.end()) {
      it.second->restart(now);
      insert(it.second);
    } else {
      // FIXME move to a free list
      delete it.second; // FIXME: no delete please
    }
  }

  if (!_timers.empty()) {
    nextExpire = _timers.begin()->second->expiration();
  }

  if (nextExpire.valid()) {
    resetTimerfd(_timerfd, nextExpire);
  }
}

bool TimerQueue::insert(Timer *timer) {
  _loop->assertInLoopThread();
  assert(_timers.size() == _activeTimers.size());
  bool earliestChanged = false;
  Timestamp when = timer->expiration();
  auto it = _timers.begin();
  if (it == _timers.end() || when < it->first) {
    earliestChanged = true;
  }
  {
    std::pair<TimerList::iterator, bool> result
        = _timers.insert(Entry(when, timer));
    assert(result.second);
    (void)result;
  }
  {
    std::pair<ActiveTimerSet::iterator, bool> result
        = _activeTimers.insert(ActiveTimer(timer, timer->sequence()));
    assert(result.second);
    (void)result;
  }

  assert(_timers.size() == _activeTimers.size());
  return earliestChanged;
}
