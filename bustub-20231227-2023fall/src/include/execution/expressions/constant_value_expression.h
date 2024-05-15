//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// constant_value_expression.h
//
// Identification: src/include/expression/constant_value_expression.h
//
// Copyright (c) 2015-19, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
//这段代码定义了一个名为 ConstantValueExpression 的类，用于表示常量值表达式。
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "execution/expressions/abstract_expression.h"

namespace bustub {
/**
 * ConstantValueExpression 表示常量。
 */
class ConstantValueExpression : public AbstractExpression {
 public:
  /** 创建一个包装给定值的新常量值表达式。 */
  explicit ConstantValueExpression(const Value &val) : AbstractExpression({}, val.GetTypeId()), val_(val) {}

  auto Evaluate(const Tuple *tuple, const Schema &schema) const -> Value override { return val_; }

  auto EvaluateJoin(const Tuple *left_tuple, const Schema &left_schema, const Tuple *right_tuple,
                    const Schema &right_schema) const -> Value override {
    return val_;
  }

  /** @return 计划节点及其子节点的字符串表示形式 */
  auto ToString() const -> std::string override { return val_.ToString(); }

  BUSTUB_EXPR_CLONE_WITH_CHILDREN(ConstantValueExpression);

  Value val_;
};
}  // namespace bustub
