
#include "buffer/buffer_pool_manager.h"
#include <utility>

#include "common/config.h"
#include "common/macros.h"
#include "storage/page/page_guard.h"

namespace bustub {

BufferPoolManager::BufferPoolManager(size_t pool_size, DiskManager *disk_manager, size_t replacer_k,
                                     LogManager *log_manager)
    : pool_size_(pool_size), disk_scheduler_(std::make_unique<DiskScheduler>(disk_manager)), log_manager_(log_manager) {
  // we allocate a consecutive memory space for the buffer pool
  pages_ = new Page[pool_size_];
  replacer_ = std::make_unique<LRUKReplacer>(pool_size, replacer_k);

  // Initially, every page is in the free list.
  for (size_t i = 0; i < pool_size_; ++i) {
    free_list_.emplace_back(static_cast<int>(i));
  }
}

BufferPoolManager::~BufferPoolManager() { delete[] pages_; }

auto BufferPoolManager::NewPage(page_id_t *page_id) -> Page * {
  std::scoped_lock<std::mutex> lock(latch_);
  frame_id_t replacement_frame_id;
  if (!free_list_.empty()) {
    replacement_frame_id = free_list_.front();
    free_list_.pop_front();
  } else {
    if (!replacer_->Evict(&replacement_frame_id)) {
      page_id = nullptr;
      return nullptr;
    }
    auto &helper = pages_[replacement_frame_id];
    if (helper.IsDirty()) {
      auto promise = disk_scheduler_->CreatePromise();
      auto future = promise.get_future();
      disk_scheduler_->Schedule({true, helper.GetData(), helper.page_id_, std::move(promise)});
      future.get();
    }
    page_table_.erase(helper.page_id_);
  }
  auto new_page_id = AllocatePage();
  *page_id = new_page_id;
  pages_[replacement_frame_id].ResetMemory();
  pages_[replacement_frame_id].pin_count_ = 0;
  pages_[replacement_frame_id].is_dirty_ = false;
  page_table_.insert(std::make_pair(new_page_id, replacement_frame_id));
  replacer_->SetEvictable(replacement_frame_id, false);
  replacer_->RecordAccess(replacement_frame_id);
  pages_[replacement_frame_id].pin_count_++;
  pages_[replacement_frame_id].page_id_ = new_page_id;

  return &pages_[replacement_frame_id];
}
auto BufferPoolManager::FetchPage(page_id_t page_id, AccessType access_type) -> Page * {
  std::scoped_lock<std::mutex> lock(latch_);
  auto iter = page_table_.find(page_id);
  if (iter != page_table_.end()) {
    auto frame_id = iter->second;
    replacer_->SetEvictable(frame_id, false);
    replacer_->RecordAccess(frame_id, access_type);
    pages_[iter->second].pin_count_++;
    return &pages_[iter->second];
  }
  frame_id_t replacement_frame_id;
  if (!free_list_.empty()) {
    replacement_frame_id = free_list_.front();
    free_list_.pop_front();
  } else {
    if (!replacer_->Evict(&replacement_frame_id)) {
      return nullptr;
    }
    auto &helper = pages_[replacement_frame_id];
    if (helper.IsDirty()) {
      auto promise1 = disk_scheduler_->CreatePromise();
      auto future1 = promise1.get_future();
      disk_scheduler_->Schedule({true, helper.data_, helper.page_id_, std::move(promise1)});
      future1.get();
    }
    page_table_.erase(helper.page_id_);
  }
  pages_[replacement_frame_id].ResetMemory();
  pages_[replacement_frame_id].pin_count_ = 0;
  pages_[replacement_frame_id].is_dirty_ = false;
  auto promise2 = disk_scheduler_->CreatePromise();
  auto future2 = promise2.get_future();
  disk_scheduler_->Schedule({false, pages_[replacement_frame_id].GetData(), page_id, std::move(promise2)});
  page_table_.insert(std::make_pair(page_id, replacement_frame_id));
  replacer_->SetEvictable(replacement_frame_id, false);
  replacer_->RecordAccess(replacement_frame_id, access_type);
  pages_[replacement_frame_id].pin_count_++;
  pages_[replacement_frame_id].page_id_ = page_id;

  future2.get();

  return pages_ + replacement_frame_id;
}

auto BufferPoolManager::UnpinPage(page_id_t page_id, bool is_dirty, [[maybe_unused]] AccessType access_type) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);
  auto itr = page_table_.find(page_id);
  if (itr == page_table_.end() || pages_[itr->second].pin_count_ <= 0) {
    return false;
  }
  if ((--pages_[itr->second].pin_count_) == 0) {
    replacer_->SetEvictable(itr->second, true);
  }
  if (is_dirty) {
    pages_[itr->second].is_dirty_ = is_dirty;
  }
  return true;
}

auto BufferPoolManager::FlushPage(page_id_t page_id) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);
  BUSTUB_ASSERT(page_id != INVALID_PAGE_ID, "page_id is invalid");
  auto itr = page_table_.find(page_id);
  if (itr == page_table_.end()) {
    return false;
  }
  auto target_id = itr->second;
  auto promise = disk_scheduler_->CreatePromise();
  auto future = promise.get_future();
  disk_scheduler_->Schedule({true, pages_[target_id].data_, pages_[target_id].page_id_, std::move(promise)});
  future.get();
  pages_[target_id].is_dirty_ = false;
  return true;
}
void BufferPoolManager::FlushAllPages() {
  std::scoped_lock<std::mutex> lock(latch_);
  for (auto &pair : page_table_) {
    page_id_t page_id = pair.first;
    BUSTUB_ASSERT(page_id != INVALID_PAGE_ID, "page_id is invalid");
    auto iter = page_table_.find(page_id);
    auto target_id = iter->second;
    auto promise = disk_scheduler_->CreatePromise();
    auto future = promise.get_future();
    disk_scheduler_->Schedule({true, pages_[target_id].data_, pages_[target_id].page_id_, std::move(promise)});
    future.get();
    pages_[target_id].is_dirty_ = false;
  }
}

auto BufferPoolManager::DeletePage(page_id_t page_id) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);
  auto iter = page_table_.find(page_id);
  if (iter == page_table_.end()) {
    return true;
  }
  auto target_id = iter->second;
  if (pages_[target_id].pin_count_ > 0) {
    return false;
  }

  page_table_.erase(page_id);
  free_list_.push_back(target_id);
  pages_[target_id].ResetMemory();
  // reset metadata of the page
  pages_[target_id].page_id_ = INVALID_PAGE_ID;
  pages_[target_id].pin_count_ = 0;
  pages_[target_id].is_dirty_ = false;

  DeallocatePage(page_id);

  return true;
}

auto BufferPoolManager::AllocatePage() -> page_id_t { return next_page_id_++; }

auto BufferPoolManager::FetchPageBasic(page_id_t page_id) -> BasicPageGuard {
  auto pg_ptr = FetchPage(page_id);
  return {this, pg_ptr};
}

auto BufferPoolManager::FetchPageRead(page_id_t page_id) -> ReadPageGuard {
  auto pg_ptr = FetchPage(page_id);
  pg_ptr->RLatch();
  assert(pg_ptr != nullptr);
  return {this, pg_ptr};
}

auto BufferPoolManager::FetchPageWrite(page_id_t page_id) -> WritePageGuard {
  auto pg_ptr = FetchPage(page_id);
  pg_ptr->WLatch();
  assert(pg_ptr != nullptr);
  return {this, pg_ptr};
}

auto BufferPoolManager::NewPageGuarded(page_id_t *page_id) -> BasicPageGuard {
  auto pg_ptr = NewPage(page_id);
  assert(pg_ptr != nullptr);
  return {this, pg_ptr};
}

}  // namespace bustub