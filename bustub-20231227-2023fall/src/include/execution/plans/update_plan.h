//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// update_plan.h
//
// Identification: src/include/execution/plans/update_plan.h
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "catalog/catalog.h"
#include "execution/expressions/abstract_expression.h"
#include "execution/plans/abstract_plan.h"

namespace bustub {

/**
 * UpdatePlanNode 标识应该更新的表。
 * 待更新的元组来自 UpdateExecutor 的子节点。
 */
class UpdatePlanNode : public AbstractPlanNode {
 public:
  /**
   * 构造一个新的 UpdatePlanNode 实例。
   * @param child 获取元组的子计划
   * @param table_oid 应该更新的表的标识符
   * @param target_expressions 新元组的目标表达式
   */
  UpdatePlanNode(SchemaRef output, AbstractPlanNodeRef child, table_oid_t table_oid,
                 std::vector<AbstractExpressionRef> target_expressions)
      : AbstractPlanNode(std::move(output), {std::move(child)}),
        table_oid_{table_oid},
        target_expressions_(std::move(target_expressions)) {}

  /** @return 计划节点的类型 */
  auto GetType() const -> PlanType override { return PlanType::Update; }

  /** @return 应该更新的表的标识符 */
  auto GetTableOid() const -> table_oid_t { return table_oid_; }

  /** @return 提供待插入元组的子计划 */
  auto GetChildPlan() const -> AbstractPlanNodeRef {
    BUSTUB_ASSERT(GetChildren().size() == 1, "UPDATE 应该有一个子计划。");
    return GetChildAt(0);
  }

  BUSTUB_PLAN_NODE_CLONE_WITH_CHILDREN(UpdatePlanNode);

  /** 待更新的表 */
  table_oid_t table_oid_;

  /** 每列的新表达式 */
  std::vector<AbstractExpressionRef> target_expressions_;

 protected:
  auto PlanNodeToString() const -> std::string override;
};

}  // namespace bustub
