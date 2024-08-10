#include "ThreadPool.h"

#include <utility>
#include <cassert>

using namespace zyweb;

ThreadPool::ThreadPool(std::string nameArg) : _mutex(),
                                              _notEmpty(_mutex),
                                              _notFull(_mutex),
                                              _name(std::move(nameArg)),
                                              _maxQueueSize(0),
                                              _running(false) {
}

ThreadPool::~ThreadPool() {
  if (_running) {
    stop();
  }
}

void ThreadPool::start(int numThreads) {
  assert(_threads.empty());
  _running = true;
  _threads.reserve(numThreads);
  for (int i = 0; i < numThreads; ++i) {
    char id[32];
    snprintf(id, sizeof id, "%d", i + 1);
    _threads.emplace_back(new Thread(
        [this] { runInThread(); }, _name + id));
    _threads[i]->start();
  }

  if (numThreads == 0 && _threadInitCallback) {
    _threadInitCallback();
  }

}

void ThreadPool::stop() {
  {
    MutexLockGuard guard(_mutex);
    _running = false;
    _notEmpty.notifyAll();
    _notFull.notifyAll();
  }

  for (auto &thr : _threads) {
    thr->join();
  }
}

size_t ThreadPool::queueSize() const {
  MutexLockGuard gard(_mutex);
  return _queue.size();
}

void ThreadPool::run(ThreadPool::Task task) {
  if (_threads.empty()) {
    task();
  } else {
    MutexLockGuard lock(_mutex);
    while (isFull() && _running) {
      _notFull.wait();
    }

    if (!_running) return;

    assert(!isFull());

    _queue.push_back(std::move(task));
    _notEmpty.notify();
  }
}

bool ThreadPool::isFull() const {
  _mutex.assertLocked();
  return _queue.size() == _maxQueueSize;
}

void ThreadPool::runInThread() {
  try {
    if (_threadInitCallback)
      _threadInitCallback();
    while (_running) {
      Task task(take());
      if (task)
        task();
    }
  }
  catch (const Exception &ex) {
    fprintf(stderr, "exception caught in ThreadPool %s\n", _name.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
    abort();
  }
  catch (const std::exception &ex) {
    fprintf(stderr, "exception caught in ThreadPool %s\n", _name.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    abort();
  }
  catch (...) {
    fprintf(stderr, "unknown exception caught in ThreadPool %s\n", _name.c_str());
    throw; // rethrow
  }
}

ThreadPool::Task ThreadPool::take() {
  MutexLockGuard lock(_mutex);
  // always use a while-loop, due to spurious wakeup
  while (_queue.empty() && _running) {
    _notEmpty.wait();
  }
  Task task;
  if (!_queue.empty()) {
    task = _queue.front();
    _queue.pop_front();
    if (_maxQueueSize > 0) {
      _notFull.notify();
    }
  }
  return task;
}
