//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// disk_manager.h
//
// Identification: src/include/storage/disk/disk_manager.h
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <atomic>
#include <fstream>
#include <future>  // NOLINT
#include <mutex>   // NOLINT
#include <string>

#include "common/config.h"

namespace bustub {

/**
 * DiskManager 负责在数据库中分配和释放页面。它执行页面到磁盘的读写操作，提供了数据库管理系统内部的逻辑文件层。
 */
class DiskManager {
 public:
  /**
   * 创建一个新的磁盘管理器，写入指定的数据库文件。
   * @param db_file 要写入的数据库文件的文件名
   */
  explicit DiskManager(const std::string &db_file);

  /** 仅用于测试/排行榜，由 DiskManagerMemory 使用 */
  DiskManager() = default;

  virtual ~DiskManager() = default;

  /**
   * 关闭磁盘管理器并关闭所有文件资源。
   */
  void ShutDown();

  /**
   * 将页面写入数据库文件。
   * @param page_id 页面的 ID
   * @param page_data 原始页面数据
   */
  virtual void WritePage(page_id_t page_id, const char *page_data);

  /**
   * 从数据库文件读取页面。
   * @param page_id 页面的 ID
   * @param[out] page_data 输出缓冲区
   */
  virtual void ReadPage(page_id_t page_id, char *page_data);

  /**
   * 将整个日志缓冲区刷新到磁盘。
   * @param log_data 原始日志数据
   * @param size 日志条目的大小
   */
  void WriteLog(char *log_data, int size);

  /**
   * 从日志文件读取日志条目。
   * @param[out] log_data 输出缓冲区
   * @param size 日志条目的大小
   * @param offset 文件中日志条目的偏移量
   * @return 如果读取成功，则为 true，否则为 false
   */
  auto ReadLog(char *log_data, int size, int offset) -> bool;

  /** @return 磁盘刷新的次数 */
  auto GetNumFlushes() const -> int;

  /** @return 如果内存内容尚未刷新，则为 true */
  auto GetFlushState() const -> bool;

  /** @return 磁盘写入的次数 */
  auto GetNumWrites() const -> int;

  /**
   * 设置用于检查非阻塞刷新的 future。
   * @param f 用于非阻塞刷新检查的 future
   */
  inline void SetFlushLogFuture(std::future<void> *f) { flush_log_f_ = f; }

  /** 检查是否设置了非阻塞刷新 future。 */
  inline auto HasFlushLogFuture() -> bool { return flush_log_f_ != nullptr; }

 protected:
  auto GetFileSize(const std::string &file_name) -> int;
  // 用于写入日志文件的流
  std::fstream log_io_;
  std::string log_name_;
  // 用于写入数据库文件的流
  std::fstream db_io_;
  std::string file_name_;
  int num_flushes_{0};
  int num_writes_{0};
  bool flush_log_{false};
  std::future<void> *flush_log_f_{nullptr};
  // 对于多个缓冲池实例，需要保护文件访问
  std::mutex db_io_latch_;
};

}  // namespace bustub
