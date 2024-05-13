

#pragma once

#include <string>
#include <utility>

#include "catalog/catalog.h"
#include "execution/expressions/abstract_expression.h"
#include "execution/plans/abstract_plan.h"

namespace bustub {

/**
 * DeletePlanNode 标识应该从中删除元组的表。
 * 要更新的元组来自 DeleteExecutor 的子项。
 *
 * 注意：为了简化任务，DeletePlanNode 最多只能有一个子项。
 */
class DeletePlanNode : public AbstractPlanNode {
 public:
  /**
   * 构造一个新的 DeletePlanNode。
   * @param child 获取元组的子计划
   * @param table_oid 应从中删除元组的表的标识符
   */
  DeletePlanNode(SchemaRef output, AbstractPlanNodeRef child, table_oid_t table_oid)
      : AbstractPlanNode(std::move(output), {std::move(child)}), table_oid_{table_oid} {}

  /** @return 计划节点的类型 */
  auto GetType() const -> PlanType override { return PlanType::Delete; }

  /** @return 应从中删除元组的表的标识符 */
  auto GetTableOid() const -> table_oid_t { return table_oid_; }

  /** @return 提供要删除的元组的子计划 */
  auto GetChildPlan() const -> AbstractPlanNodeRef {
    BUSTUB_ASSERT(GetChildren().size() == 1, "delete should have at most one child plan.");
    return GetChildAt(0);
  }

  BUSTUB_PLAN_NODE_CLONE_WITH_CHILDREN(DeletePlanNode);

  /** 应从中删除元组的表的标识符 */
  table_oid_t table_oid_;

 protected:
  auto PlanNodeToString() const -> std::string override { return fmt::format("Delete {{ table_oid={} }}", table_oid_); }
};

}  // namespace bustub
