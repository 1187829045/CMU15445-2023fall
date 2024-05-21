//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// column_value_expression.h
//
// Identification: src/include/expression/column_value_expression.h
//
// Copyright (c) 2015-19, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
//这段代码定义了一个名为 ColumnValueExpression 的类，用于表示基于元组索引和列索引的列值表达式
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "catalog/schema.h"                             // 包含表结构的头文件
#include "execution/expressions/abstract_expression.h"  // 包含抽象表达式的头文件
#include "storage/table/tuple.h"                        // 包含元组的头文件

namespace bustub {
/**
 * ColumnValueExpression维护相对于特定模式或连接的元组索引和列索引。
 */
class ColumnValueExpression : public AbstractExpression {
 public:
  /**
   * ColumnValueExpression是基于索引的“Table.member”的抽象表示。
   * @param tuple_idx {元组索引0 =连接的左侧，元组索引1 =连接的右侧}
   * @param col_idx 元组模式中列的索引
   * @param ret_type 表达式的返回类型
   */
  ColumnValueExpression(uint32_t tuple_idx, uint32_t col_idx, TypeId ret_type)
      : AbstractExpression({}, ret_type), tuple_idx_{tuple_idx}, col_idx_{col_idx} {}
  //提供对表达式的评估：Evaluate 方法用于在给定元组和模式的情况下计算表达式的值。根据提供的元组和模式，通过调用元组的
  //GetValue 方法获取相应列的值。
  auto Evaluate(const Tuple *tuple, const Schema &schema) const -> Value override {
    return tuple->GetValue(&schema, col_idx_);
  }
  //提供对连接操作的评估：EvaluateJoin 方法用于在给定左侧和右侧连接的元组、模式的情况下计算表达式的值。
  //根据提供的左侧和右侧连接的元组、模式，通过调用左侧或右侧元组的 GetValue 方法获取相应列的值。
  auto EvaluateJoin(const Tuple *left_tuple, const Schema &left_schema, const Tuple *right_tuple,
                    const Schema &right_schema) const -> Value override {
    return tuple_idx_ == 0 ? left_tuple->GetValue(&left_schema, col_idx_)
                           : right_tuple->GetValue(&right_schema, col_idx_);
  }

  auto GetTupleIdx() const -> uint32_t { return tuple_idx_; }
  auto GetColIdx() const -> uint32_t { return col_idx_; }

  /** @return 计划节点及其子节点的字符串表示 */
  auto ToString() const -> std::string override { return fmt::format("#{}.{}", tuple_idx_, col_idx_); }

  BUSTUB_EXPR_CLONE_WITH_CHILDREN(ColumnValueExpression);

 private:
  /** 元组索引0 =连接的左侧，元组索引1 =连接的右侧 */
  uint32_t tuple_idx_;
  /** 列索引指的是元组模式中的索引，例如，模式 {A,B,C} 有索引 {0,1,2} */
  uint32_t col_idx_;
};
}  // namespace bustub
