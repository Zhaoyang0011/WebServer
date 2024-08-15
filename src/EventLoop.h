#ifndef EVENTLOOP_H_
#define EVENTLOOP_H_

#include <functional>
#include <any>
#include <atomic>
#include <memory>
#include "base/NonCopyable.h"
#include "base/Timestamp.h"
#include "util/Mutex.h"
#include "TimerId.h"

namespace zyweb::net {

class Channel;
class Poller;
class TimerQueue;

class EventLoop : NonCopyable {

 public:
  typedef std::function<void()> Functor;

  EventLoop();
  ~EventLoop();  // force out-line dtor, for std::unique_ptr members.

  ///
  /// Loops forever.
  ///
  /// Must be called in the same thread as creation of the object.
  ///
  void loop();

  /// Quits loop.
  ///
  /// This is not 100% thread safe, if you call through a raw pointer,
  /// better to call through shared_ptr<EventLoop> for 100% safety.
  void quit();

  ///
  /// Time when poll returns, usually means data arrival.
  ///
  Timestamp pollReturnTime() const { return _pollReturnTime; }

  int64_t iteration() const { return _iteration; }

  /// Runs callback immediately in the loop thread.
  /// It wakes up the loop, and run the cb.
  /// If in the same loop thread, cb is run within the function.
  /// Safe to call from other threads.
  void runInLoop(Functor cb);
  /// Queues callback in the loop thread.
  /// Runs after finish pooling.
  /// Safe to call from other threads.
  void queueInLoop(Functor cb);

  size_t queueSize() const;

  // timers

  ///
  /// Runs callback at 'time'.
  /// Safe to call from other threads.
  ///
  TimerId runAt(Timestamp time, TimerCallback cb);
  ///
  /// Runs callback after @c delay seconds.
  /// Safe to call from other threads.
  ///
  TimerId runAfter(double delay, TimerCallback cb);
  ///
  /// Runs callback every @c interval seconds.
  /// Safe to call from other threads.
  ///
  TimerId runEvery(double interval, TimerCallback cb);
  ///
  /// Cancels the timer.
  /// Safe to call from other threads.
  ///
  void cancel(TimerId timerId);

  // internal usage
  void wakeup();
  void updateChannel(Channel *channel);
  void removeChannel(Channel *channel);
  bool hasChannel(Channel *channel);

  pid_t threadId() const { return _threadId; }
  void assertInLoopThread() {
    if (!isInLoopThread()) {
      abortNotInLoopThread();
    }
  }
  bool isInLoopThread() const { return _threadId == CurrentThread::tid(); }
  // bool callingPendingFunctors() const { return callingPendingFunctors_; }
  bool eventHandling() const { return _eventHandling; }

  void setContext(const std::any &context) { _context = context; }

  const std::any &getContext() const { return _context; }

  std::any *getMutableContext() { return &_context; }

  static EventLoop *getEventLoopOfCurrentThread();

 private:
  void abortNotInLoopThread();
  void handleRead();  // waked up
  void doPendingFunctors();

  void printActiveChannels() const; // DEBUG

  typedef std::vector<Channel *> ChannelList;

  bool _looping; /* atomic */
  std::atomic<bool> _quit;
  bool _eventHandling; /* atomic */
  bool _callingPendingFunctors; /* atomic */
  int64_t _iteration;
  const pid_t _threadId;
  Timestamp _pollReturnTime;
  std::unique_ptr<Poller> _poller;
  std::unique_ptr<TimerQueue> _timerQueue;
  int _wakeupFd;
  // unlike in TimerQueue, which is an internal class,
  // we don't expose Channel to client.
  std::unique_ptr<Channel> _wakeupChannel;
  std::any _context;

  // scratch variables
  ChannelList _activeChannels;
  Channel *_currentActiveChannel;

  mutable MutexLock _mutex;
  std::vector<Functor> _pendingFunctors;
};

}

#endif
