#include <cassert>
#include <utility>
#include "Thread.h"

void *startThread(void *obj) {
  auto *data = static_cast<zyweb::ThreadData *>(obj);
  data->runInThread();
  delete data;
  return nullptr;
}

zyweb::Thread::Thread(ThreadFunc func, std::string name) :
    _started(false),
    _joined(false),
    _pthreadId(0),
    _tid(0),
    _func(std::move(func)),
    _name(std::move(name)),
    _latch(1) {

}
zyweb::Thread::~Thread() {
  if (_started && !_joined) {
    pthread_detach(_pthreadId);
  }
}
void zyweb::Thread::start() {
  assert(!_started);
  _started = true;
  // FIXME: move(func_)
  auto *data = new ThreadData(_func, _name, &_tid, &_latch);
  if (pthread_create(&_pthreadId, nullptr, &startThread, data)) {
    _started = false;
    delete data;
//    LOG_SYSFATAL << "Failed in pthread_create";
  } else {
    _latch.wait();
    assert(_tid > 0);
  }
}

int zyweb::Thread::join() {
  assert(_started);
  assert(!_joined);
  _joined = true;
  return pthread_join(_pthreadId, nullptr);
}

void zyweb::Thread::setDefaultName() {
  int num = ++_numCreated;
  if (_name.empty()) {
    char buf[32];
    snprintf(buf, sizeof buf, "Thread%d", num);
    _name = buf;
  }
}
