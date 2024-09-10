#ifndef COROUTINE_H_
#define COROUTINE_H_

#include <functional>
#include <cstdint>
#include <memory>
#include "coroutine/fcontext/fcontext.h"

namespace zyweb {

class CoroutineScheduler;

class Coroutine : public std::enable_shared_from_this<Coroutine> {
 public:
  /**
   * @brief 协程状态
   */
  enum State {
    /// 初始化状态
    INIT,
    /// 暂停状态
    HOLD,
    /// 执行中状态
    EXEC,
    /// 结束状态
    TERM,
    /// 可执行状态
    READY,
    /// 异常状态
    EXCEPT
  };
 private:
  /**
   * @brief 无参构造函数
   * @attention 每个线程第一个协程的构造
   */
  Coroutine();

  /**
   * @brief 构造函数
   * @param[in] cb 协程执行的函数
   * @param[in] stacksize 协程栈大小
   * @param[in] use_caller 是否在MainCoroutine上调度
   */
  explicit Coroutine(std::function<void()> cb, size_t stacksize = 0, bool use_caller = false);

 public:
  /**
   * @brief 析构函数
   */
  ~Coroutine();

  /**
   * @brief 重置协程执行函数,并设置状态
   * @pre getState() 为 INIT, TERM, EXCEPT
   * @post getState() = INIT
   */
  void reset(std::function<void()> cb);

  /**
   * @brief 将当前协程切换到运行状态
   * @pre getState() != EXEC
   * @post getState() = EXEC
   */
  void swapIn();

  /**
   * @brief 将当前协程切换到后台
   */
  void swapOut();

  /**
   * @brief 将当前线程切换到执行状态
   * @pre 执行的为当前线程的主协程
   */
  void resume();

  /**
   * @brief 将当前线程切换到后台
   * @pre 执行的为该协程
   * @post 返回到线程的主协程
   */
  void yield();

  /**
   * @brief 返回协程id
   */
  uint64_t getId() const { return id_; }

  /**
   * @brief 返回协程状态
   */
  State getState() const { return state_; }
 public:

  static Coroutine *NewCoroutine();
  static Coroutine *NewCoroutine(std::function<void()>, size_t, bool);

  /**
   * @brief 设置当前线程的运行协程
   * @param[in] f 运行协程
   */
  static void SetThis(Coroutine *f);

  /**
   * @brief 返回当前所在的协程
   */
  static std::shared_ptr<Coroutine> GetThis();

  /**
   * @brief 将当前协程切换到后台,并设置为READY状态
   * @post getState() = READY
   */
  static void YieldToReady();

  /**
   * @brief 将当前协程切换到后台,并设置为HOLD状态
   * @post getState() = HOLD
   */
  static void YieldToHold();

  /**
   * @brief 返回当前协程的总数量
   */
  static uint64_t TotalCoroutines();

  /**
   * @brief 协程执行函数
   * @post 执行完成返回到线程主协程
   */
  static void MainFunc(intptr_t vp);

  /**
   * @brief 协程执行函数
   * @post 执行完成返回到线程调度协程
   */
  static void CallerMainFunc(intptr_t vp);

  static uint64_t GetCurrentId();
 private:
  /// 协程id
  uint64_t id_ = 0;
  /// 协程运行栈大小
  uint32_t stack_size_ = 0;
  /// 协程状态
  State state_ = INIT;
  /// 协程运行函数
  std::function<void()> cb_;
  /// 协程上下文
  fcontext_t ctx_ = nullptr;
  /// 协程运行栈指针
  char stack_[];
};

}

#endif
