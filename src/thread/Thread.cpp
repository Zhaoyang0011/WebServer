#include "Thread.h"

zyweb::Thread::Thread(zyweb::Thread::ThreadFunc func, const std::string &name) : _started(false),
                                                                            _joined(false),
                                                                            _pthreadId(0),
                                                                            _tid(0),
                                                                            _func(std::move(func)),
                                                                            _name(name),
                                                                            _latch(1) {

}
zyweb::Thread::~Thread() {

}
void zyweb::Thread::start() {

}
int zyweb::Thread::join() {
  return 0;
}
