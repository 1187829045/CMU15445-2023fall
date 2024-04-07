#include "storage/disk/disk_scheduler.h"
#include "common/exception.h"
#include "storage/disk/disk_manager.h"

namespace bustub {
DiskScheduler::DiskScheduler(DiskManager *disk_manager) : disk_manager_(disk_manager) {
  // TODO(P1): 在实现磁盘调度器 API 后移除此行
  throw NotImplementedException(
      "DiskScheduler 尚未实现。如果您已经完成了磁盘调度器的实现，请在 `disk_scheduler.cpp` 中删除 throw 异常的代码行。");

  // 创建后台线程
  background_thread_.emplace([&] { StartWorkerThread(); });
}

DiskScheduler::~DiskScheduler() {
  // 将 `std::nullopt` 放入队列以表示退出循环
  request_queue_.Put(std::nullopt);
  if (background_thread_.has_value()) {
    background_thread_->join();
  }
}

void DiskScheduler::Schedule(DiskRequest r) {
  
}

void DiskScheduler::StartWorkerThread() {

}

}  // namespace bustub
