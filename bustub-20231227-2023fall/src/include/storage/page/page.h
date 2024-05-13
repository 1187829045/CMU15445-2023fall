

#pragma once

#include <cstring>
#include <iostream>

#include "common/config.h"
#include "common/rwlatch.h"

namespace bustub {

/**
 * Page 是数据库系统中存储的基本单位。Page 包装了实际存储在主内存中的数据页。Page
 * 还包含了缓冲池管理器使用的书记信息，例如 锁定计数、脏标志、页面 ID 等。
 */
class Page {
  // Page 内部有一些只与缓冲池管理器相关的书记信息。
  friend class BufferPoolManager;

 public:
  /** 构造函数。将页面数据清零。 */
  Page() {
    data_ = new char[BUSTUB_PAGE_SIZE];
    ResetMemory();
  }

  /** 默认析构函数。 */
  ~Page() { delete[] data_; }

  /** @return 此页面包含的实际数据 */
  inline auto GetData() -> char * { return data_; }

  /** @return 此页面的页面 ID */
  inline auto GetPageId() -> page_id_t { return page_id_; }

  /** @return 此页面的锁定计数 */
  inline auto GetPinCount() -> int { return pin_count_; }

  /** @return 如果内存中的页面已从磁盘上的页面修改，则返回 true，否则返回 false */
  inline auto IsDirty() -> bool { return is_dirty_; }

  /** 获取页面写锁。 */
  inline void WLatch() { rwlatch_.WLock(); }

  /** 释放页面写锁。 */
  inline void WUnlatch() { rwlatch_.WUnlock(); }

  /** 获取页面读锁。 */
  inline void RLatch() { rwlatch_.RLock(); }

  /** 释放页面读锁。 */
  inline void RUnlatch() { rwlatch_.RUnlock(); }

  /** @return 页面的LSN。 */
  inline auto GetLSN() -> lsn_t { return *reinterpret_cast<lsn_t *>(GetData() + OFFSET_LSN); }

  /** 设置页面的LSN。 */
  inline void SetLSN(lsn_t lsn) { memcpy(GetData() + OFFSET_LSN, &lsn, sizeof(lsn_t)); }

 protected:
  static_assert(sizeof(page_id_t) == 4);
  static_assert(sizeof(lsn_t) == 4);

  static constexpr size_t SIZE_PAGE_HEADER = 8;
  static constexpr size_t OFFSET_PAGE_START = 0;
  static constexpr size_t OFFSET_LSN = 4;

 private:
  /** 将页面中的数据清零。 */
  inline void ResetMemory() { memset(data_, OFFSET_PAGE_START, BUSTUB_PAGE_SIZE); }

  /** 实际存储在页面中的数据。 */
  // 通常应该存储为 `char data_[BUSTUB_PAGE_SIZE]{};`。但为了使 ASAN 能够检测到页面溢出，我们将其存储为指针。
  char *data_;
  /** 此页的ID。 */
  page_id_t page_id_ = INVALID_PAGE_ID;
  int pin_count_ = 0; /** 此页的锁定计数。 */
  /** 如果页面是脏的，则为 true，即它与磁盘上的对应页面不同。 */
  bool is_dirty_ = false;
  /** 页面锁。 */
  ReaderWriterLatch rwlatch_;
};

}  // namespace bustub
