//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// table_page.h
//
// Identification: src/include/storage/page/table_page.h
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <cstring>
#include <optional>
#include <tuple>
#include <utility>

#include "common/config.h"
#include "common/rid.h"
#include "concurrency/lock_manager.h"
#include "recovery/log_manager.h"
#include "storage/page/page.h"
#include "storage/table/table_heap.h"
#include "storage/table/tuple.h"

namespace bustub {

static constexpr uint64_t TABLE_PAGE_HEADER_SIZE = 8;  // table_page_header_size

/**
 * 开槽页面格式：
 *  ---------------------------------------------------------
 *  | HEADER | ... FREE SPACE ... | ... INSERTED TUPLES ... |           header   free space    inserted tuples
 *  ---------------------------------------------------------
 *                                ^
 *                                free space pointer
 *
 *  Header format (size in bytes):
 *  ----------------------------------------------------------------------------
 *  | NextPageId (4)| NumTuples(2) | NumDeletedTuples(2) |
 *  ----------------------------------------------------------------------------
 *  ----------------------------------------------------------------
 *  | Tuple_1 offset+size (4) | Tuple_2 offset+size (4) | ... |
 *  ----------------------------------------------------------------
 *
 * Tuple format:
 * | meta | data |
 */

class TablePage {
 public:
  /**
   * 初始化表页头部。
   */
  void Init();

  /** @return 页中的元组数量 */
  auto GetNumTuples() const -> uint32_t { return num_tuples_; }

  /** @return 表中下一个表页的页面ID */
  auto GetNextPageId() const -> page_id_t { return next_page_id_; }

  /** 设置表中下一页的页面ID。 */
  void SetNextPageId(page_id_t next_page_id) { next_page_id_ = next_page_id; }

  /** 获取下一个要插入的偏移量，如果该元组无法适合在此页中，则返回nullopt */
  auto GetNextTupleOffset(const TupleMeta &meta, const Tuple &tuple) const -> std::optional<uint16_t>;

  /**
   * 向表中插入一个元组。
   * @param tuple 要插入的元组
   * @return 如果插入成功（即有足够的空间），则返回true
   */
  auto InsertTuple(const TupleMeta &meta, const Tuple &tuple) -> std::optional<uint16_t>;

  /**
   * 更新一个元组。
   */
  void UpdateTupleMeta(const TupleMeta &meta, const RID &rid);

  /**
   * 从表中读取一个元组。
   */
  auto GetTuple(const RID &rid) const -> std::pair<TupleMeta, Tuple>;

  /**
   * 从表中读取一个元组的元信息。
   */
  auto GetTupleMeta(const RID &rid) const -> TupleMeta;

  /**
   * 在原地更新一个元组。
   */
  void UpdateTupleInPlaceUnsafe(const TupleMeta &meta, const Tuple &tuple, RID rid);

  static_assert(sizeof(page_id_t) == 4);

 private:
  using TupleInfo = std::tuple<uint16_t, uint16_t, TupleMeta>;
  char page_start_[0];
  page_id_t next_page_id_;
  uint16_t num_tuples_;
  uint16_t num_deleted_tuples_;
  TupleInfo tuple_info_[0];

  static constexpr size_t TUPLE_INFO_SIZE = 24;
  static_assert(sizeof(TupleInfo) == TUPLE_INFO_SIZE);
};

static_assert(sizeof(TablePage) == TABLE_PAGE_HEADER_SIZE);

}  // namespace bustub
