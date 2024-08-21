#ifndef COROUTINESCHEDULER_H_
#define COROUTINESCHEDULER_H_

#include <list>
#include <vector>
#include <atomic>
#include <memory>
#include "base/types.h"
#include "util/Mutex.h"
#include "thread/ThreadPool.h"

namespace zyweb {

class Coroutine;

class CoroutineScheduler {
 public:

  /**
   * @brief 构造函数
   * @param[in] threads 线程数量
   * @param[in] use_caller 是否使用当前调用线程
   * @param[in] name 协程调度器名称
   */
  explicit CoroutineScheduler(size_t threads = 1, bool use_caller = true, const std::string &name = "");

  /**
   * @brief 析构函数
   */
  virtual ~CoroutineScheduler();

  /**
   * @brief 返回协程调度器名称
   */
  const string &getName() const { return _name; }

  /**
   * @brief 启动协程调度器
   */
  void start();

  /**
   * @brief 停止协程调度器
   */
  void stop();

 protected:
  friend class Coroutine;
  /**
   * @brief 通知协程调度器有任务了
   */
  virtual void tickle();

  /**
   * @brief 协程调度函数
   */
  void run();

  /**
   * @brief 返回是否可以停止
   */
  virtual bool stopping();

  /**
   * @brief 协程无任务可调度时执行idle协程
   */
  virtual void idle();

  /**
   * @brief 设置当前的协程调度器
   */
  void setThis();

  /**
   * @brief 是否有空闲线程
   */
  bool hasIdleThreads() const { return _idleCoroutineCount > 0; }

  /**
   * @brief 返回当前协程调度器
   */
  static CoroutineScheduler *GetThis();

  /**
   * @brief 返回当前协程调度器的调度协程
   */
  static Coroutine *GetMainCoroutine();
 private:
  /// Mutex
  MutexLock _mutex;
  /// 线程池
  ThreadPool _threadPool;
  /// 待执行的协程队列
  std::list<CoroutineData> _coroutines;
  /// use_caller为true时有效, 调度协程
  std::shared_ptr<CoroutineScheduler> _rootFiber;
  /// 协程调度器名称
  std::string _name;
 protected:
  /// 协程下的线程id数组
  std::vector<int> _threadIds;
  /// 线程数量
  size_t _coroutineCount = 0;
  /// 工作线程数量
  std::atomic<size_t> _activeCoroutineCount = {0};
  /// 空闲线程数量
  std::atomic<size_t> _idleCoroutineCount = {0};
  /// 是否正在停止
  bool _stopping = true;
  /// 是否自动停止
  bool _autoStop = false;
  /// 主线程id(use_caller)
  int _rootCoroutine = 0;
};

}

#endif
