#include "storage/disk/disk_scheduler.h"
#include "common/exception.h"
#include "storage/disk/disk_manager.h"

namespace bustub {
DiskScheduler::DiskScheduler(DiskManager *disk_manager) : disk_manager_(disk_manager) {
  // 启动后台线程
  background_thread_.emplace([&] { StartWorkerThread(); });
}

DiskScheduler::~DiskScheduler() {
  // Put a `std::nullopt` in the queue to signal to exit the loop
  request_queue_.Put(std::nullopt);
  if (background_thread_.has_value()) {
    background_thread_->join();
  }
}
void DiskScheduler::Schedule(DiskRequest r) { request_queue_.Put(std::move(r)); }

void DiskScheduler::StartWorkerThread() {
  std::optional<DiskRequest> request;
  while ((request = request_queue_.Get())) {
    if (request) {
      if (request->is_write_) {
        disk_manager_->WritePage(request->page_id_, request->data_);
        request->callback_.set_value(true);
        continue;
      }
      disk_manager_->ReadPage(request->page_id_, request->data_);
      request->callback_.set_value(true);
    }
  }
}

}  // namespace bustub