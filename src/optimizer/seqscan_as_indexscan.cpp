#include "execution/expressions/column_value_expression.h"
#include "execution/expressions/comparison_expression.h"
#include "execution/plans/index_scan_plan.h"
#include "execution/plans/seq_scan_plan.h"
#include "optimizer/optimizer.h"

namespace bustub {

// 优化器函数：将序列扫描计划优化为索引扫描计划
auto Optimizer::OptimizeSeqScanAsIndexScan(const bustub::AbstractPlanNodeRef &plan) -> AbstractPlanNodeRef {
  // 遍历子计划，递归优化
  std::vector<AbstractPlanNodeRef> children;
  for (const auto &child : plan->GetChildren()) {
    children.emplace_back(OptimizeSeqScanAsIndexScan(child));
  }
  // 克隆计划，用优化后的子计划替换原有的子计划
  auto optimized_plan = plan->CloneWithChildren(std::move(children));

  // 如果计划类型为序列扫描
  if (optimized_plan->GetType() == PlanType::SeqScan) {
    // 动态类型转换为序列扫描计划节点
    if (const auto &seq_scan = dynamic_cast<const SeqScanPlanNode &>(*optimized_plan);
        seq_scan.filter_predicate_ != nullptr) {
      // 如果过滤谓词不为空
      // 动态类型转换为比较表达式
      if (const auto *cmp_expr = dynamic_cast<const ComparisonExpression *>(seq_scan.filter_predicate_.get());
          cmp_expr != nullptr && cmp_expr->comp_type_ == ComparisonType::Equal) {
        // 如果是相等比较表达式
        // 获取表格信息
        const auto *table_info = catalog_.GetTable(seq_scan.GetTableOid());
        // 获取表格的所有索引
        const auto indices = catalog_.GetTableIndexes(table_info->name_);
        // 动态类型转换为列值表达式
        const auto *column_value_expr = dynamic_cast<ColumnValueExpression *>(cmp_expr->children_[0].get());

        // 遍历所有索引
        for (const auto *index : indices) {
          // 获取索引的列
          const auto &columns = index->index_->GetKeyAttrs();
          // 获取过滤列的ID
          std::vector<uint32_t> filter_column_ids = {column_value_expr->GetColIdx()};
          // 如果过滤列ID与索引列相匹配
          if (filter_column_ids == columns) {
            // 返回索引扫描计划节点
            return std::make_shared<IndexScanPlanNode>(optimized_plan->output_schema_, table_info->oid_,
                                                       index->index_oid_, seq_scan.filter_predicate_);
          }
        }
      }
    }
  }
  // 返回优化后的计划
  return optimized_plan;
}

}  // namespace bustub
