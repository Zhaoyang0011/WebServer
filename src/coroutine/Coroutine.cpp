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
  state_ = EXEC;
  SetThis(this);
  ++coroutine_count;
}

Coroutine::Coroutine(std::function<void()> cb, size_t stacksize, bool use_caller)
    : id_(++coroutine_id),
      cb_(std::move(cb)) {
  ++coroutine_count;
  stack_size_ = stacksize;
  if (!use_caller) {
    ctx_ = make_fcontext((char *)stack_ + stack_size_, stack_size_, &Coroutine::MainFunc);
  } else {
    ctx_ = make_fcontext((char *)stack_ + stack_size_, stack_size_, &Coroutine::CallerMainFunc);
  }
}

Coroutine::~Coroutine() {
  --coroutine_count;
  if (stack_size_) {
    assert(state_ == TERM || state_ == EXCEPT || state_ == INIT);
  } else {
    assert(!cb_);
    assert(state_ == EXEC);

    Coroutine *cur = cur_coroutine;
    if (cur == this) {
      SetThis(nullptr);
    }
  }
}

void Coroutine::reset(std::function<void()> cb) {
  assert(stack_);
  assert(state_ == TERM || state_ == EXCEPT || state_ == INIT);
  cb_ = std::move(cb);

  ctx_ = make_fcontext((char *)stack_ + stack_size_, stack_size_, &MainFunc);

  state_ = INIT;
}

void Coroutine::swapIn() {
  SetThis(this);
  assert(state_ != EXEC);
  state_ = EXEC;
  jump_fcontext(&thread_coroutine->ctx_, ctx_, 0);
}

void Coroutine::swapOut() {
  SetThis(thread_coroutine.get());
  jump_fcontext(&ctx_, thread_coroutine->ctx_, 0);
}

void Coroutine::resume() {
  SetThis(this);
  state_ = EXEC;
  jump_fcontext(&thread_coroutine->ctx_, ctx_, 0);
}

void Coroutine::yield() {
  SetThis(thread_coroutine.get());
  jump_fcontext(&ctx_, thread_coroutine->ctx_, 0);
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
  NewCoroutine();
  std::shared_ptr<Coroutine> main_fiber(cur_coroutine);
  assert(cur_coroutine == main_fiber.get());
  thread_coroutine = main_fiber;
  return cur_coroutine->shared_from_this();
}

void Coroutine::YieldToReady() {
  auto cur = GetThis();
  assert(cur->state_ == EXEC);
  cur->state_ = READY;
  cur->swapOut();
}

void Coroutine::YieldToHold() {
  auto cur = GetThis();
  assert(cur->state_ == EXEC);
  cur->state_ = HOLD;
  cur->swapOut();
}

uint64_t Coroutine::TotalCoroutines() {
  return coroutine_count;
}

void Coroutine::MainFunc(intptr_t vp) {
  auto cur = GetThis();
  assert(cur);
  try {
    cur->cb_();
    cur->cb_ = nullptr;
    cur->state_ = TERM;
  } catch (std::exception &ex) {
    cur->state_ = EXCEPT;
  } catch (...) {
    cur->state_ = EXCEPT;
  }
  auto raw_ptr = cur.get();
  cur.reset();
  raw_ptr->yield();
  assert(false);
}

void Coroutine::CallerMainFunc(intptr_t vp) {
  auto cur = GetThis();
  assert(cur);
  try {
    cur->cb_();
    cur->cb_ = nullptr;
    cur->state_ = TERM;
  } catch (std::exception &ex) {
    cur->state_ = EXCEPT;
  } catch (...) {
    cur->state_ = EXCEPT;
  }

  auto raw_ptr = cur.get();
  cur.reset();
  raw_ptr->yield();
}
