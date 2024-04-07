#pragma once
#include <future>  // NOLINT
#include <optional>
#include <thread>  // NOLINT
#include "common/channel.h"
#include "storage/disk/disk_manager.h"
namespace bustub {
/**
 * @brief 表示DiskManager要执行的Write或Read请求。
 */
struct DiskRequest {
  /** 标志指示请求是写入还是读取。 */
  bool is_write_;

  /**
   * 指向内存位置开头的指针，其中一个页面正在：
   *   1. 从磁盘读入（在读取时）。
   *   2. 写入到磁盘（在写入时）。
   */
  char *data_;

  /** 从磁盘读取/写入的页面的ID。 */
  page_id_t page_id_;//page_id_t 是一个自定义的数据类型，用于表示页面的唯一标识符或页面ID。通常情况下，它是一个整数类型

  /** 用于向请求发起者发出信号，表示请求已完成的回调。 */
  std::promise<bool> callback_;//std::promise<bool> 是 C++ 标准库中的一个模板类
  //，用于支持异步任务的同步操作。std::promise 类提供了一种机制，允许异步任务在执行完成后设置一个值，然后其他线程可以等待这个值的设定
};

/**
 * @brief DiskScheduler调度磁盘读写操作。
 *
 * 通过使用适当的DiskRequest对象调用DiskScheduler::Schedule()来安排请求。调度程序
 * 维护一个后台工作线程，该线程使用磁盘管理器处理已安排的请求。在DiskScheduler的构造函数中创建后台
 * 线程，在其析构函数中加入。
 */
class DiskScheduler {
 public:
  explicit DiskScheduler(DiskManager *disk_manager);
  ~DiskScheduler();

  /**
   * @brief 安排DiskManager执行的请求。
   *
   * @param r 要安排的请求。
   */
  void Schedule(DiskRequest r);

  /**
   * @brief 后台工作线程函数，处理已安排的请求。
   *
   * 后台线程需要在DiskScheduler存在时处理请求，即此函数不应返回，直到调用~DiskScheduler()为止。
   * 在那时，您需要确保函数返回。
   */
  void StartWorkerThread();

  using DiskSchedulerPromise = std::promise<bool>;//定义别名

  /**
   * @brief 创建一个Promise对象。如果您想实现自己版本的promise，可以更改此函数，以便我们的测试用例可以使用您的promise实现。
   *
   * @return std::promise<bool>
   */
  auto CreatePromise() -> DiskSchedulerPromise { return {}; };

 private:
  /** 指向磁盘管理器的指针。 */
  DiskManager *disk_manager_ __attribute__((__unused__));
  /** 一个共享队列，用于并发安排和处理请求。当调用DiskScheduler的析构函数时，会将`std::nullopt`放入队列中，以向后台线程发出停止执行的信号。 */
  Channel<std::optional<DiskRequest>> request_queue_;
  /** 负责将已安排的请求提交给磁盘管理器的后台线程。 */
  std::optional<std::thread> background_thread_;
};
}  // namespace bustub
