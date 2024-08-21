#include <atomic>
#include <utility>
#include <cassert>
#include "Coroutine.h"
//#include "CoroutineScheduler.h"

using namespace zyweb;

static uint32_t co_stack_size = 128 * 1024;

static std::atomic<uint64_t> coroutine_id{0};
static std::atomic<uint64_t> coroutine_count{0};

static thread_local Coroutine *cur_coroutine = nullptr;
static thread_local std::shared_ptr<Coroutine> thread_coroutine = nullptr;

uint64_t Coroutine::GetCurrentId() {
  if (cur_coroutine) {
    return cur_coroutine->getId();
  }
  return 0;
}

Coroutine::Coroutine() {
  _state = EXEC;
  SetThis(this);
  ++coroutine_count;
}

Coroutine::Coroutine(std::function<void()> cb, size_t stacksize, bool use_caller)
    : _id(++coroutine_id),
      _cb(std::move(cb)) {
  ++coroutine_count;
  _stacksize = stacksize;
  if (!use_caller) {
    _ctx = make_fcontext((char *)_stack + _stacksize, _stacksize, &Coroutine::MainFunc);
  } else {
    _ctx = make_fcontext((char *)_stack + _stacksize, _stacksize, &Coroutine::CallerMainFunc);
  }
}

Coroutine::~Coroutine() {
  --coroutine_count;
  if (_stacksize) {
    assert(_state == TERM || _state == EXCEPT || _state == INIT);
  } else {
    assert(!_cb);
    assert(_state == EXEC);

    Coroutine *cur = cur_coroutine;
    if (cur == this) {
      SetThis(nullptr);
    }
  }
}

void Coroutine::reset(std::function<void()> cb) {
  assert(_stack);
  assert(_state == TERM || _state == EXCEPT || _state == INIT);
  _cb = std::move(cb);

  _ctx = make_fcontext((char *)_stack + _stacksize, _stacksize, &MainFunc);

  _state = INIT;
}

void Coroutine::swapIn() {
  SetThis(this);
  assert(_state != EXEC);
  _state = EXEC;
  jump_fcontext(&thread_coroutine->_ctx, _ctx, 0);
}

void Coroutine::swapOut() {
  SetThis(thread_coroutine.get());
  assert(_state != EXEC);
  _state = EXEC;
  jump_fcontext(&_ctx, thread_coroutine->_ctx, 0);
}

void Coroutine::resume() {
  SetThis(this);
  _state = EXEC;
  jump_fcontext(&thread_coroutine->_ctx, _ctx, 0);
}

void Coroutine::yield() {
  SetThis(thread_coroutine.get());
  jump_fcontext(&_ctx, thread_coroutine->_ctx, 0);
}

Coroutine *Coroutine::NewCoroutine() {
  return new Coroutine();
}

Coroutine *Coroutine::NewCoroutine(std::function<void()> cb, size_t stacksize, bool use_caller) {
  stacksize = stacksize ? stacksize : co_stack_size;
  auto *p = (Coroutine *)malloc(sizeof(Coroutine) + stacksize);
  return new(p) Coroutine(std::move(cb), stacksize, use_caller);
}

void Coroutine::SetThis(Coroutine *f) {
  cur_coroutine = f;
}

std::shared_ptr<Coroutine> Coroutine::GetThis() {
  if (cur_coroutine) {
    return cur_coroutine->shared_from_this();
  }
  std::shared_ptr<Coroutine> main_fiber(NewCoroutine());
  thread_coroutine = main_fiber;
  return cur_coroutine->shared_from_this();
}

void Coroutine::YieldToReady() {
  auto cur = GetThis();
  assert(cur->_stacksize == EXEC);
  cur->_stacksize = READY;
  cur->swapOut();
}

void Coroutine::YieldToHold() {
  auto cur = GetThis();
  assert(cur->_stacksize == EXEC);
  cur->_stacksize = HOLD;
  cur->swapOut();
}

uint64_t Coroutine::TotalCoroutines() {
  return coroutine_count;
}

void Coroutine::MainFunc(intptr_t vp) {
  auto cur = GetThis();
  assert(cur);
  try {
    cur->_cb();
    cur->_cb = nullptr;
    cur->_state = TERM;
  } catch (std::exception &ex) {
    cur->_state = EXCEPT;
  } catch (...) {
    cur->_state = EXCEPT;
  }
  auto raw_ptr = cur.get();
  cur.reset();
  raw_ptr->swapOut();
  assert(false);
}

void Coroutine::CallerMainFunc(intptr_t vp) {
  auto cur = GetThis();
  assert(cur);
  try {
    cur->_cb();
    cur->_cb = nullptr;
    cur->_state = TERM;
  } catch (std::exception &ex) {
    cur->_state = EXCEPT;
  } catch (...) {
    cur->_state = EXCEPT;
  }

  auto raw_ptr = cur.get();
  cur.reset();
  raw_ptr->yield();
}
