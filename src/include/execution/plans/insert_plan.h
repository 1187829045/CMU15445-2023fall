
#pragma once

#include <string>
#include <utility>
#include <vector>

#include "catalog/catalog.h"
#include "execution/expressions/abstract_expression.h"
#include "execution/plans/abstract_plan.h"

namespace bustub {

/**
 * InsertPlanNode 标识要插入元组的表。
 *
 * 将要插入的值将来自节点的子节点。
 */
class InsertPlanNode : public AbstractPlanNode {
 public:
  /**
   * 为从子计划插入值创建新的插入计划节点。
   * @param child 从中获取值的子计划
   * @param table_oid 应插入的表的标识符
   */
  InsertPlanNode(SchemaRef output, AbstractPlanNodeRef child, table_oid_t table_oid)
      : AbstractPlanNode(std::move(output), {std::move(child)}), table_oid_(table_oid) {}

  /** @return 计划节点的类型 */
  auto GetType() const -> PlanType override { return PlanType::Insert; }

  /** @return 要插入元组的表的标识符 */
  auto GetTableOid() const -> table_oid_t { return table_oid_; }

  /** @return 提供要插入的元组的子计划 */
  auto GetChildPlan() const -> AbstractPlanNodeRef {
    BUSTUB_ASSERT(GetChildren().size() == 1, "Insert should have only one child plan.");
    return GetChildAt(0);
  }

  BUSTUB_PLAN_NODE_CLONE_WITH_CHILDREN(InsertPlanNode);

  /** 要插入的表。 */
  table_oid_t table_oid_;

 protected:
  auto PlanNodeToString() const -> std::string override { return fmt::format("Insert {{ table_oid={} }}", table_oid_); }
};

}  // namespace bustub
