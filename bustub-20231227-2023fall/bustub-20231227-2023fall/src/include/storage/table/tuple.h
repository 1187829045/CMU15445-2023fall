//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// tuple.h
//
// 标识: src/include/storage/table/tuple.h
//
// 版权所有（c）2015-2019，卡内基梅隆大学数据库组
//
//===----------------------------------------------------------------------===//

#pragma once

#include <string>
#include <vector>

#include "catalog/schema.h"
#include "common/config.h"
#include "common/rid.h"
#include "type/value.h"

namespace bustub {

using timestamp_t = int64_t;
const timestamp_t INVALID_TS = -1;

static constexpr size_t TUPLE_META_SIZE = 16;

/** 元组的元信息 */
struct TupleMeta {
  /** 元组的时间戳/事务ID。在项目3中，只需将其设置为0。 */
  timestamp_t ts_;
  /** 标记此元组是否已从表堆中删除。 */
  bool is_deleted_;

  friend auto operator==(const TupleMeta &a, const TupleMeta &b) {
    return a.ts_ == b.ts_ && a.is_deleted_ == b.is_deleted_;
  }

  friend auto operator!=(const TupleMeta &a, const TupleMeta &b) { return !(a == b); }
};

static_assert(sizeof(TupleMeta) == TUPLE_META_SIZE);

/**
 * 元组格式:
 * ---------------------------------------------------------------------
 * | 固定大小或可变大小的偏移量 | 可变大小字段的有效载荷 |
 * ---------------------------------------------------------------------
 */
class Tuple {
  friend class TablePage;
  friend class TableHeap;
  friend class TableIterator;

 public:
  // 默认构造函数（创建一个虚拟元组）
  Tuple() = default;

  // 用于表堆元组的构造函数
  explicit Tuple(RID rid) : rid_(rid) {}

  static auto Empty() -> Tuple { return Tuple{RID{INVALID_PAGE_ID, 0}}; }

  // 基于输入值创建新元组的构造函数
  Tuple(std::vector<Value> values, const Schema *schema);

  Tuple(const Tuple &other) = default;

  // 移动构造函数
  Tuple(Tuple &&other) noexcept = default;

  // 分配运算符，深度复制
  auto operator=(const Tuple &other) -> Tuple & = default;

  // 移动分配
  auto operator=(Tuple &&other) noexcept -> Tuple & = default;

  // 序列化元组数据
  void SerializeTo(char *storage) const;

  // 反序列化元组数据（深度复制）
  void DeserializeFrom(const char *storage);

  // 返回当前元组的RID
  inline auto GetRid() const -> RID { return rid_; }

  // 设置当前元组的RID
  inline auto SetRid(RID rid) { rid_ = rid; }

  // 获取元组在表的后端存储中的地址
  inline auto GetData() const -> const char * { return data_.data(); }

  // 获取元组的长度，包括varchar长度
  inline auto GetLength() const -> uint32_t { return data_.size(); }

  // 获取指定列的值（const）
  // 检查模式以查看如何返回值。
  auto GetValue(const Schema *schema, uint32_t column_idx) const -> Value;

  // 根据模式和属性生成键元组
  auto KeyFromTuple(const Schema &schema, const Schema &key_schema, const std::vector<uint32_t> &key_attrs) -> Tuple;

  // 列值是否为空？
  inline auto IsNull(const Schema *schema, uint32_t column_idx) const -> bool {
    Value value = GetValue(schema, column_idx);
    return value.IsNull();
  }

  auto ToString(const Schema *schema) const -> std::string;

  friend inline auto IsTupleContentEqual(const Tuple &a, const Tuple &b) { return a.data_ == b.data_; }

 private:
  // 获取特定列的起始存储地址
  auto GetDataPtr(const Schema *schema, uint32_t column_idx) const -> const char *;

  RID rid_{};  // 如果指向表堆，则RID有效
  std::vector<char> data_;
};

}  // namespace bustub
