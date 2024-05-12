//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// abstract_expression.h
//
// Identification: src/include/expression/abstract_expression.h
//
// 版权所有 (c) 2015-2021，卡内基梅隆大学数据库组
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "catalog/schema.h"
#include "fmt/format.h"
#include "storage/table/tuple.h"

#define BUSTUB_EXPR_CLONE_WITH_CHILDREN(cname)                                                                   \
  auto CloneWithChildren(std::vector<AbstractExpressionRef> children) const->std::unique_ptr<AbstractExpression> \
      override {                                                                                                 \
    auto expr = cname(*this);                                                                                    \
    expr.children_ = children;                                                                                   \
    return std::make_unique<cname>(std::move(expr));                                                             \
  }

namespace bustub {

class AbstractExpression;
using AbstractExpressionRef = std::shared_ptr<AbstractExpression>;

/**
 * AbstractExpression 是系统中所有表达式的基类。
 * 表达式被建模为树形结构，即每个表达式可能具有可变数量的子表达式。
 */
class AbstractExpression {
 public:
  /**
   * 创建一个具有给定子表达式和返回类型的新 AbstractExpression。
   * @param children 此抽象表达式的子表达式
   * @param ret_type 当对其进行评估时此抽象表达式的返回类型
   */
  AbstractExpression(std::vector<AbstractExpressionRef> children, TypeId ret_type)
      : children_{std::move(children)}, ret_type_{ret_type} {}

  /** 虚析构函数。 */
  virtual ~AbstractExpression() = default;

  /** @return 通过评估具有给定模式的元组获得的值 */
  virtual auto Evaluate(const Tuple *tuple, const Schema &schema) const -> Value = 0;

  /**
   * 返回通过 JOIN 评估获得的值。
   * @param left_tuple 左元组
   * @param left_schema 左元组的模式
   * @param right_tuple 右元组
   * @param right_schema 右元组的模式
   * @return 在左和右上评估 JOIN 获得的值
   */
  virtual auto EvaluateJoin(const Tuple *left_tuple, const Schema &left_schema, const Tuple *right_tuple,
                            const Schema &right_schema) const -> Value = 0;

  /** @return 此表达式的 child_idx 个子表达式 */
  auto GetChildAt(uint32_t child_idx) const -> const AbstractExpressionRef & { return children_[child_idx]; }

  /** @return 此表达式的子表达式，顺序可能重要 */
  auto GetChildren() const -> const std::vector<AbstractExpressionRef> & { return children_; }

  /** @return 如果对其进行评估，则此表达式的类型 */
  virtual auto GetReturnType() const -> TypeId { return ret_type_; }

  /** @return 计划节点及其子节点的字符串表示 */
  virtual auto ToString() const -> std::string { return "<未知>"; }

  /** @return 带有新子表达式的新表达式 */
  virtual auto CloneWithChildren(std::vector<AbstractExpressionRef> children) const
      -> std::unique_ptr<AbstractExpression> = 0;

  /** 此表达式的子表达式。请注意，子表达式的出现顺序可能重要。 */
  std::vector<AbstractExpressionRef> children_;

 private:
  /** 此表达式的返回类型。 */
  TypeId ret_type_;
};

}  // namespace bustub

template <typename T>
struct fmt::formatter<T, std::enable_if_t<std::is_base_of<bustub::AbstractExpression, T>::value, char>>
    : fmt::formatter<std::string> {
  template <typename FormatCtx>
  auto format(const T &x, FormatCtx &ctx) const {
    return fmt::formatter<std::string>::format(x.ToString(), ctx);
  }
};

template <typename T>
struct fmt::formatter<std::unique_ptr<T>, std::enable_if_t<std::is_base_of<bustub::AbstractExpression, T>::value, char>>
    : fmt::formatter<std::string> {
  template <typename FormatCtx>
  auto format(const std::unique_ptr<T> &x, FormatCtx &ctx) const {
    if (x != nullptr) {
      return fmt::formatter<std::string>::format(x->ToString(), ctx);
    }
    return fmt::formatter<std::string>::format("", ctx);
  }
};

template <typename T>
struct fmt::formatter<std::shared_ptr<T>, std::enable_if_t<std::is_base_of<bustub::AbstractExpression, T>::value, char>>
    : fmt::formatter<std::string> {
  template <typename FormatCtx>
  auto format(const std::shared_ptr<T> &x, FormatCtx &ctx) const {
    if (x != nullptr) {
      return fmt::formatter<std::string>::format(x->ToString(), ctx);
    }
    return fmt::formatter<std::string>::format("", ctx);
  }
};
