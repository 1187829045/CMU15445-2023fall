//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// schema.h
//
// Identification: src/include/catalog/schema.h
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "catalog/column.h"
#include "common/exception.h"
#include "type/type.h"

namespace bustub {

class Schema;
using SchemaRef = std::shared_ptr<const Schema>;

// 数据库表模式类
class Schema {
 public:
  /**
   * 构造函数，根据给定的列向量构造模式。
   * @param columns 描述模式的列
   */
  explicit Schema(const std::vector<Column> &columns);

  // 从另一个模式复制一份
  static auto CopySchema(const Schema *from, const std::vector<uint32_t> &attrs) -> Schema {
    std::vector<Column> cols;
    cols.reserve(attrs.size());
    for (const auto i : attrs) {
      cols.emplace_back(from->columns_[i]);
    }
    return Schema{cols};
  }

  /** @return 模式中的所有列 */
  auto GetColumns() const -> const std::vector<Column> & { return columns_; }

  /**
   * 返回模式中的特定列。
   * @param col_idx 请求列的索引
   * @return 请求的列
   */
  auto GetColumn(const uint32_t col_idx) const -> const Column & { return columns_[col_idx]; }

  /**
   * 查找并返回模式中具有指定名称的第一列的索引。
   * 如果有多列具有相同的名称，则返回第一个这样的索引。
   * @param col_name 要查找的列的名称
   * @return 具有给定名称的列的索引，如果不存在则抛出异常
   */
  auto GetColIdx(const std::string &col_name) const -> uint32_t {
    if (auto col_idx = TryGetColIdx(col_name)) {
      return *col_idx;
    }
    UNREACHABLE("列不存在");
  }

  /**
   * 查找并返回模式中具有指定名称的第一列的索引。
   * 如果有多列具有相同的名称，则返回第一个这样的索引。
   * @param col_name 要查找的列的名称
   * @return 具有给定名称的列的索引，如果不存在则返回 std::nullopt
   */
  auto TryGetColIdx(const std::string &col_name) const -> std::optional<uint32_t> {
    for (uint32_t i = 0; i < columns_.size(); ++i) {
      if (columns_[i].GetName() == col_name) {
        return std::optional{i};
      }
    }
    return std::nullopt;
  }

  /** @return 非内联列的索引 */
  auto GetUnlinedColumns() const -> const std::vector<uint32_t> & { return uninlined_columns_; }

  /** @return 元组中模式的列数 */
  auto GetColumnCount() const -> uint32_t { return static_cast<uint32_t>(columns_.size()); }

  /** @return 非内联列的数量 */
  auto GetUnlinedColumnCount() const -> uint32_t { return static_cast<uint32_t>(uninlined_columns_.size()); }

  /** @return 一个元组使用的字节数 */
  inline auto GetLength() const -> uint32_t { return length_; }

  /** @return 如果所有列都是内联的则返回 true，否则返回 false */
  inline auto IsInlined() const -> bool { return tuple_is_inlined_; }

  /** @return 返回该模式的字符串表示形式 */
  auto ToString(bool simplified = true) const -> std::string;

 private:
  /** 固定长度列的大小，即一个元组使用的字节数 */
  uint32_t length_;

  /** 模式中的所有列，包括内联列和非内联列 */
  std::vector<Column> columns_;

  /** 如果所有列都是内联的，则为 true，否则为 false */
  bool tuple_is_inlined_{true};

  /** 所有非内联列的索引 */
  std::vector<uint32_t> uninlined_columns_;
};

}  // namespace bustub

// 定义 bustub::Schema 的格式化器
template <typename T>
struct fmt::formatter<T, std::enable_if_t<std::is_base_of<bustub::Schema, T>::value, char>>
    : fmt::formatter<std::string> {
  template <typename FormatCtx>
  auto format(const bustub::Schema &x, FormatCtx &ctx) const {
    return fmt::formatter<std::string>::format(x.ToString(), ctx);
  }
};

// 定义 std::shared_ptr<bustub::Schema> 的格式化器
template <typename T>
struct fmt::formatter<std::shared_ptr<T>, std::enable_if_t<std::is_base_of<bustub::Schema, T>::value, char>>
    : fmt::formatter<std::string> {
  template <typename FormatCtx>
  auto format(const std::shared_ptr<T> &x, FormatCtx &ctx) const {
    if (x != nullptr) {
      return fmt::formatter<std::string>::format(x->ToString(), ctx);
    }
    return fmt::formatter<std::string>::format("", ctx);
  }
};

// 定义 std::unique_ptr<bustub::Schema> 的格式化器
template <typename T>
struct fmt::formatter<std::unique_ptr<T>, std::enable_if_t<std::is_base_of<bustub::Schema, T>::value, char>>
    : fmt::formatter<std::string> {
  template <typename FormatCtx>
  auto format(const std::unique_ptr<T> &x, FormatCtx &ctx) const {
    if (x != nullptr) {
      return fmt::formatter<std::string>::format(x->ToString(), ctx);
    }
    return fmt::formatter<std::string>::format("", ctx);
  }
};