#ifndef CHANNEL_H_
#define CHANNEL_H_

#include <functional>
#include <memory>
#include "base/NonCopyable.h"
#include "base/Timestamp.h"

namespace zyweb::net {

class EventLoop;

class Channel : NonCopyable {
 public:
  typedef std::function<void()> EventCallback;
  typedef std::function<void(Timestamp)> ReadEventCallback;

  Channel(EventLoop *loop, int fd);
  ~Channel();

  void handleEvent(Timestamp receiveTime);
  void setReadCallback(ReadEventCallback cb) { _readCallback = std::move(cb); }
  void setWriteCallback(EventCallback cb) { _writeCallback = std::move(cb); }
  void setCloseCallback(EventCallback cb) { _closeCallback = std::move(cb); }
  void setErrorCallback(EventCallback cb) { _errorCallback = std::move(cb); }

  /// Tie this channel to the owner object managed by shared_ptr,
  /// prevent the owner object being destroyed in handleEvent.
  void tie(const std::shared_ptr<void> &);

  int fd() const { return _fd; }
  int events() const { return _events; }
  void set_revents(int revt) { _revents = revt; } // used by pollers
  // int revents() const { return r_events; }
  bool isNoneEvent() const { return _events == kNoneEvent; }

  void enableReading() {
    _events |= kReadEvent;
    update();
  }

  void disableReading() {
    _events &= ~kReadEvent;
    update();
  }

  void enableWriting() {
    _events |= kWriteEvent;
    update();
  }

  void disableWriting() {
    _events &= ~kWriteEvent;
    update();
  }

  void disableAll() {
    _events = kNoneEvent;
    update();
  }

  bool isWriting() const { return _events & kWriteEvent; }

  bool isReading() const { return _events & kReadEvent; }

  // for Poller
  int index() const { return _index; }
  void set_index(int idx) { _index = idx; }

  // for debug
  string reventsToString() const;
  string eventsToString() const;

  void doNotLogHup() { _logHup = false; }

  EventLoop *ownerLoop() { return _loop; }
  void remove();

 private:
  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  static string eventsToString(int fd, int ev);

  void update();
  void handleEventWithGuard(Timestamp receiveTime);

  EventLoop *_loop;
  const int _fd;
  int _events;
  int _revents; // it's the received event types of epoll or poll
  int _index; // used by Poller.
  bool _logHup;

  std::weak_ptr<void> _tie;
  bool _tied;
  bool _eventHandling;
  bool _addedToLoop;
  ReadEventCallback _readCallback;
  EventCallback _writeCallback;
  EventCallback _closeCallback;
  EventCallback _errorCallback;
};

}

#endif