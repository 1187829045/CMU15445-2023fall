//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// nested_loop_join.h
//
// Identification: src/include/execution/plans/nested_loop_join.h
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "binder/table_ref/bound_join_ref.h"
#include "catalog/catalog.h"
#include "execution/expressions/abstract_expression.h"
#include "execution/plans/abstract_plan.h"
#include "fmt/core.h"

namespace bustub {

/**
 * NestedLoopJoinPlanNode从两个子计划节点中连接元组。
 */
class NestedLoopJoinPlanNode : public AbstractPlanNode {
 public:
  /**
   * 构造一个新的NestedLoopJoinPlanNode实例。
   * @param output 此嵌套循环连接节点的输出格式
   * @param children 两个连续扫描子计划
   * @param predicate 用于连接的谓词，如果谓词(tuple) = true，则元组被连接。
   */
  NestedLoopJoinPlanNode(SchemaRef output_schema, AbstractPlanNodeRef left, AbstractPlanNodeRef right,
                         AbstractExpressionRef predicate, JoinType join_type)
      : AbstractPlanNode(std::move(output_schema), {std::move(left), std::move(right)}),
        predicate_(std::move(predicate)),
        join_type_(join_type) {}

  /** @return 计划节点的类型 */
  auto GetType() const -> PlanType override { return PlanType::NestedLoopJoin; }

  /** @return 用于嵌套循环连接的谓词 */
  auto Predicate() const -> const AbstractExpressionRef & { return predicate_; }

  /** @return 在嵌套循环连接中使用的连接类型 */
  auto GetJoinType() const -> JoinType { return join_type_; };

  /** @return 嵌套循环连接的左侧计划节点，按照约定，它应该是较小的表 */
  auto GetLeftPlan() const -> AbstractPlanNodeRef { return GetChildAt(0); }

  /** @return 嵌套循环连接的右侧计划节点 */
  auto GetRightPlan() const -> AbstractPlanNodeRef { return GetChildAt(1); }

  static auto InferJoinSchema(const AbstractPlanNode &left, const AbstractPlanNode &right) -> Schema;

  BUSTUB_PLAN_NODE_CLONE_WITH_CHILDREN(NestedLoopJoinPlanNode);

  /** 连接谓词 */
  AbstractExpressionRef predicate_;

  /** 连接类型 */
  JoinType join_type_;

 protected:
  auto PlanNodeToString() const -> std::string override {
    return fmt::format("NestedLoopJoin {{ type={}, predicate={} }}", join_type_, predicate_);
  }
};

}  // namespace bustub
