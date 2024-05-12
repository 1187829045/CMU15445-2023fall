//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// table_heap.h
//
// Identification: src/include/storage/table/table_heap.h
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <mutex>  // NOLINT
#include <optional>
#include <utility>

#include "buffer/buffer_pool_manager.h"
#include "common/config.h"
#include "concurrency/lock_manager.h"
#include "concurrency/transaction.h"
#include "recovery/log_manager.h"
#include "storage/page/page_guard.h"
#include "storage/page/table_page.h"
#include "storage/table/table_iterator.h"
#include "storage/table/tuple.h"

namespace bustub {

class TablePage;

/**
 * TableHeap 表示磁盘上的物理表。
 * 这只是一个页面的双向链表。
 */
class TableHeap {
  friend class TableIterator;

 public:
  ~TableHeap() = default;

  /**
   * 创建一个没有事务的表堆。（打开表）
   * @param buffer_pool_manager 缓冲池管理器
   * @param first_page_id 第一个页面的id
   */
  explicit TableHeap(BufferPoolManager *bpm);

  /**
   * 将元组插入表中。如果元组太大（>= page_size），返回std::nullopt。
   * @param meta 元组元数据
   * @param tuple 要插入的元组
   * @return 插入的元组的rid
   */
  auto InsertTuple(const TupleMeta &meta, const Tuple &tuple, LockManager *lock_mgr = nullptr,
                   Transaction *txn = nullptr, table_oid_t oid = 0) -> std::optional<RID>;

  /**
   * 更新元组的元数据。
   * @param meta 新的元组元数据
   * @param rid 要更新的元组的rid
   */
  void UpdateTupleMeta(const TupleMeta &meta, RID rid);

  /**
   * 从表中读取元组。
   * @param rid 要读取的元组的rid
   * RID（Record Identifier）是记录标识符，用于唯一标识数据库表中的每个记录或元组。
   * 它通常由两部分组成：页面编号（Page ID）和槽号（Slot Number）。
   * @return 元数据和元组
   */
  auto GetTuple(RID rid) -> std::pair<TupleMeta, Tuple>;

  /**
   * 从表中读取元组元数据。注意：如果要获取元组和元数据，请使用`GetTuple`，以确保原子性。
   * @param rid 要读取的元组的rid
   * @return 元数据
   */
  auto GetTupleMeta(RID rid) -> TupleMeta;

  /** @return
   * 此表的迭代器。当创建此迭代器时，它将记录表堆中当前最后一个元组，并且迭代器将停在该点，以避免Halloween问题。
   * 通常情况下，您需要使用此函数进行项目 3 的实现。假设您已经将项目 4
   * 的更新执行器实现为管道断点，您可以使用`MakeEagerIterator`来测试更新执行器是否正确实现。
   * 如果一切都正确实现，则此函数和`MakeEagerIterator`在项目 4 中应该没有区别。
   */
  auto MakeIterator() -> TableIterator;

  /** @return 此表的迭代器。迭代器将停在迭代时的最后一个元组。 */
  auto MakeEagerIterator() -> TableIterator;

  /** @return 此表的第一个页面的id */
  inline auto GetFirstPageId() const -> page_id_t { return first_page_id_; }

  /**
   * 在原地更新元组。项目 3 中不应使用此功能。将您的项目 3 更新执行器实现为删除和插入。
   * 在项目 4 中，您将需要使用此功能。
   * @param meta 新的元组元数据
   * @param tuple  新的元组
   * @param rid 要更新的元组的rid
   * @param check 实际更新之前运行的检查。
   */
  auto UpdateTupleInPlace(const TupleMeta &meta, const Tuple &tuple, RID rid,
                          std::function<bool(const TupleMeta &meta, const Tuple &table, RID rid)> &&check = nullptr)
      -> bool;

  /** 供绑定器测试使用 */
  static auto CreateEmptyHeap(bool create_table_heap = false) -> std::unique_ptr<TableHeap> {
    // 输入参数应为false，以生成一个空堆
    assert(!create_table_heap);
    return std::unique_ptr<TableHeap>(new TableHeap(create_table_heap));
  }

  // 下面的函数仅在您想以一种从版本链中删除撤消日志的方式实现中止时才有用。如果不确定这些函数的作用，请勿使用它们。
  // 如果决定使用下面的函数，请勿使用常规的函数，如`GetTuple`。在一个线程中对同一对象进行两次读取锁定可能会导致死锁。

  auto AcquireTablePageReadLock(RID rid) -> ReadPageGuard;

  auto AcquireTablePageWriteLock(RID rid) -> WritePageGuard;

  void UpdateTupleInPlaceWithLockAcquired(const TupleMeta &meta, const Tuple &tuple, RID rid, TablePage *page);

  auto GetTupleWithLockAcquired(RID rid, const TablePage *page) -> std::pair<TupleMeta, Tuple>;

  auto GetTupleMetaWithLockAcquired(RID rid, const TablePage *page) -> TupleMeta;

 private:
  /** 供绑定器测试使用 */
  explicit TableHeap(bool create_table_heap = false);

  BufferPoolManager *bpm_;
  page_id_t first_page_id_{INVALID_PAGE_ID};

  std::mutex latch_;
  page_id_t last_page_id_{INVALID_PAGE_ID}; /* 受latch_保护 */
};

}  // namespace bustub
