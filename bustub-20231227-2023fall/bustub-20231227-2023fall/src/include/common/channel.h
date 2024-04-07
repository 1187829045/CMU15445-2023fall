#pragma once

#include <condition_variable>  // NOLINT
#include <mutex>               // NOLINT
#include <queue>
#include <utility>

namespace bustub {

/**
 * Channels允许在线程之间安全地共享数据。这是一个多生产者多消费者通道。
 */
template <class T>
class Channel {
 public:
  Channel() = default;
  ~Channel() = default;

  /**
   * @brief 将元素插入共享队列中。
   *
   * @param element 要插入的元素。
   */
  void Put(T element) {
    std::unique_lock<std::mutex> lk(m_);  // 获取互斥锁
    q_.push(std::move(element));  // 将元素移入队列
    lk.unlock();  // 解锁互斥锁
    cv_.notify_all();  // 通知所有等待线程，有元素可取
  }

  /**
   * @brief 从共享队列中获取一个元素。如果队列为空，则阻塞直到有元素可用。
   */
  auto Get() -> T {
    std::unique_lock<std::mutex> lk(m_);  // 获取互斥锁
    cv_.wait(lk, [&]() { return !q_.empty(); });  // 当队列不为空时，等待条件变量，否则阻塞
    T element = std::move(q_.front());  // 取出队列头部元素
    q_.pop();  // 移除队列头部元素
    return element;  // 返回取出的元素
  }

 private:
  std::mutex m_;  // 互斥锁，保护队列访问
  std::condition_variable cv_;  // 条件变量，用于线程同步
  std::queue<T> q_;  // 共享队列，存储元素
};
}  // namespace bustub
