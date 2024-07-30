#pragma once

#include <memory>
#include <string>
#include <utility>

#include "binder/table_ref/bound_base_table_ref.h"
#include "catalog/catalog.h"
#include "catalog/schema.h"
#include "execution/expressions/abstract_expression.h"
#include "execution/plans/abstract_plan.h"

namespace bustub {

/**
 * SeqScanPlanNode 表示顺序表扫描操作。
 */
class SeqScanPlanNode : public AbstractPlanNode {
 public:
  /*
   * 构造一个新的SeqScanPlanNode实例。
   * @param output 该顺序扫描计划节点的输出模式
   * @param table_oid 待扫描表的标识符
   */
  SeqScanPlanNode(SchemaRef output, table_oid_t table_oid, std::string table_name,
                  AbstractExpressionRef filter_predicate = nullptr)
      : AbstractPlanNode(std::move(output), {}),
        table_oid_{table_oid},
        table_name_(std::move(table_name)),
        filter_predicate_(std::move(filter_predicate)) {}

  /** @return 计划节点的类型 */
  auto GetType() const -> PlanType override { return PlanType::SeqScan; }

  /** @return 应该扫描的表的标识符 */
  auto GetTableOid() const -> table_oid_t { return table_oid_; }

  static auto InferScanSchema(const BoundBaseTableRef &table_ref) -> Schema;

  BUSTUB_PLAN_NODE_CLONE_WITH_CHILDREN(SeqScanPlanNode);

  /** 应该扫描其元组的表 */
  table_oid_t table_oid_;

  /** 表名 */
  std::string table_name_;

  /** 用于在顺序扫描中过滤的谓词。
   * 对于2023年秋季学期，我们将启用MergeFilterScan规则，因此我们可以进一步支持索引点查找
   */
  AbstractExpressionRef filter_predicate_;

 protected:
  auto PlanNodeToString() const -> std::string override {
    if (filter_predicate_) {
      return fmt::format("SeqScan {{ table={}, filter={} }}", table_name_, filter_predicate_);
    }
    return fmt::format("SeqScan {{ table={} }}", table_name_);
  }
};

}  // namespace bustub
