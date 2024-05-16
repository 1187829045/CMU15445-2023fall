//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// logic_expression.h
//
// Identification: src/include/expression/logic_expression.h
//
// Copyright (c) 2015-19, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "catalog/schema.h"
#include "common/exception.h"
#include "common/macros.h"
#include "execution/expressions/abstract_expression.h"
#include "fmt/format.h"
#include "storage/table/tuple.h"
#include "type/type.h"
#include "type/type_id.h"
#include "type/value_factory.h"

namespace bustub {

/** ArithmeticType 表示我们要执行的逻辑操作类型。 */
enum class LogicType { And, Or };

/**
 * LogicExpression 表示两个表达式之间的计算。
 */
class LogicExpression : public AbstractExpression {
 public:
  /**
   * 创建一个新的逻辑表达式，表示 (left logic_type right)。
   * @param left 左侧表达式
   * @param right 右侧表达式
   * @param logic_type 逻辑操作类型
   */
  LogicExpression(AbstractExpressionRef left, AbstractExpressionRef right, LogicType logic_type)
      : AbstractExpression({std::move(left), std::move(right)}, TypeId::BOOLEAN), logic_type_{logic_type} {
    if (GetChildAt(0)->GetReturnType() != TypeId::BOOLEAN || GetChildAt(1)->GetReturnType() != TypeId::BOOLEAN) {
      throw bustub::NotImplementedException("期望两侧均为布尔类型");
    }
  }

  /**
   * 对给定的元组和模式计算逻辑表达式的值。
   * @param tuple 输入元组
   * @param schema 输入元组的模式
   * @return 计算结果
   */
  auto Evaluate(const Tuple *tuple, const Schema &schema) const -> Value override {
    Value lhs = GetChildAt(0)->Evaluate(tuple, schema);
    Value rhs = GetChildAt(1)->Evaluate(tuple, schema);
    return ValueFactory::GetBooleanValue(PerformComputation(lhs, rhs));
  }

  /**
   * 对给定的左右连接元组和模式计算逻辑表达式的值。
   * @param left_tuple 左连接元组
   * @param left_schema 左连接元组的模式
   * @param right_tuple 右连接元组
   * @param right_schema 右连接元组的模式
   * @return 计算结果
   */
  auto EvaluateJoin(const Tuple *left_tuple, const Schema &left_schema, const Tuple *right_tuple,
                    const Schema &right_schema) const -> Value override {
    Value lhs = GetChildAt(0)->EvaluateJoin(left_tuple, left_schema, right_tuple, right_schema);
    Value rhs = GetChildAt(1)->EvaluateJoin(left_tuple, left_schema, right_tuple, right_schema);
    return ValueFactory::GetBooleanValue(PerformComputation(lhs, rhs));
  }

  /** @return 表达式节点及其子节点的字符串表示 */
  auto ToString() const -> std::string override {
    return fmt::format("({}{}{})", *GetChildAt(0), logic_type_, *GetChildAt(1));
  }

  BUSTUB_EXPR_CLONE_WITH_CHILDREN(LogicExpression);

  LogicType logic_type_;

 private:
  /**
   * 将布尔值转换为 CmpBool 类型。
   * @param val 布尔值
   * @return 对应的 CmpBool 值
   */
  auto GetBoolAsCmpBool(const Value &val) const -> CmpBool {
    if (val.IsNull()) {
      return CmpBool::CmpNull;
    }
    if (val.GetAs<bool>()) {
      return CmpBool::CmpTrue;
    }
    return CmpBool::CmpFalse;
  }

  /**
   * 执行逻辑运算。
   * @param lhs 左侧值
   * @param rhs 右侧值
   * @return 计算结果
   */
  auto PerformComputation(const Value &lhs, const Value &rhs) const -> CmpBool {
    auto l = GetBoolAsCmpBool(lhs);
    auto r = GetBoolAsCmpBool(rhs);
    switch (logic_type_) {
      case LogicType::And:
        if (l == CmpBool::CmpFalse || r == CmpBool::CmpFalse) {
          return CmpBool::CmpFalse;
        }
        if (l == CmpBool::CmpTrue && r == CmpBool::CmpTrue) {
          return CmpBool::CmpTrue;
        }
        return CmpBool::CmpNull;
      case LogicType::Or:
        if (l == CmpBool::CmpFalse && r == CmpBool::CmpFalse) {
          return CmpBool::CmpFalse;
        }
        if (l == CmpBool::CmpTrue || r == CmpBool::CmpTrue) {
          return CmpBool::CmpTrue;
        }
        return CmpBool::CmpNull;
      default:
        UNREACHABLE("不支持的逻辑类型。");
    }
  }
};
}  // namespace bustub

template <>
struct fmt::formatter<bustub::LogicType> : formatter<string_view> {
  template <typename FormatContext>
  auto format(bustub::LogicType c, FormatContext &ctx) const {
    string_view name;
    switch (c) {
      case bustub::LogicType::And:
        name = "and";
        break;
      case bustub::LogicType::Or:
        name = "or";
        break;
      default:
        name = "Unknown";
        break;
    }
    return formatter<string_view>::format(name, ctx);
  }
};
