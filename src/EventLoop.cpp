#include "EventLoop.h"
#include "poller/Poller.h"

using namespace zyweb;
using namespace zyweb::net;

EventLoop::EventLoop() {

}

EventLoop::~EventLoop() {

}

void EventLoop::loop() {

}

void EventLoop::quit() {

}

void EventLoop::runInLoop(EventLoop::Functor cb) {

}

void EventLoop::queueInLoop(EventLoop::Functor cb) {

}

size_t EventLoop::queueSize() const {
  return 0;
}

TimerId EventLoop::runAt(Timestamp time, TimerCallback cb) {
  return TimerId();
}

TimerId EventLoop::runAfter(double delay, TimerCallback cb) {
  return TimerId();
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb) {
  return TimerId();
}

void EventLoop::cancel(TimerId timerId) {

}

void EventLoop::wakeup() {

}

void EventLoop::updateChannel(Channel *channel) {

}

void EventLoop::removeChannel(Channel *channel) {

}

bool EventLoop::hasChannel(Channel *channel) {
  return false;
}

EventLoop *EventLoop::getEventLoopOfCurrentThread() {
  return nullptr;
}

void EventLoop::abortNotInLoopThread() {

}

void EventLoop::handleRead() {

}

void EventLoop::doPendingFunctors() {

}

void EventLoop::printActiveChannels() const {

}
