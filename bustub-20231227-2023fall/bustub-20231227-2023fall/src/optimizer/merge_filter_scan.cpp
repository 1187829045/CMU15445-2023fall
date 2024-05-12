#include <memory>
#include <vector>
#include "execution/plans/filter_plan.h"
#include "execution/plans/limit_plan.h"
#include "execution/plans/seq_scan_plan.h"
#include "execution/plans/sort_plan.h"
#include "execution/plans/topn_plan.h"

#include "optimizer/optimizer.h"

namespace bustub {

// 优化器类的函数，用于优化合并过滤扫描
auto Optimizer::OptimizeMergeFilterScan(const AbstractPlanNodeRef &plan) -> AbstractPlanNodeRef {
  // 存储优化后的子计划
  std::vector<AbstractPlanNodeRef> children;
  // 对子计划递归进行优化
  for (const auto &child : plan->GetChildren()) {
    children.emplace_back(OptimizeMergeFilterScan(child));
  }

  // 使用优化后的子计划构造新的计划
  auto optimized_plan = plan->CloneWithChildren(std::move(children));

  // 如果优化后的计划类型为过滤器
  if (optimized_plan->GetType() == PlanType::Filter) {
    // 强制转换为 FilterPlanNode 类型，获取过滤器计划的信息
    const auto &filter_plan = dynamic_cast<const FilterPlanNode &>(*optimized_plan);
    // 确保过滤器计划有且只有一个子计划
    BUSTUB_ASSERT(optimized_plan->children_.size() == 1, "must have exactly one children");
    // 获取过滤器计划的子计划信息
    const auto &child_plan = *optimized_plan->children_[0];
    // 如果子计划类型为顺序扫描
    if (child_plan.GetType() == PlanType::SeqScan) {
      // 强制转换为 SeqScanPlanNode 类型，获取顺序扫描计划的信息
      const auto &seq_scan_plan = dynamic_cast<const SeqScanPlanNode &>(child_plan);
      // 如果顺序扫描计划没有过滤谓词
      if (seq_scan_plan.filter_predicate_ == nullptr) {
        // 构造一个新的顺序扫描计划，加入过滤谓词
        return std::make_shared<SeqScanPlanNode>(filter_plan.output_schema_, seq_scan_plan.table_oid_,
                                                 seq_scan_plan.table_name_, filter_plan.GetPredicate());
      }
    }
  }

  return optimized_plan; // 返回优化后的计划
}

}  // namespace bustub
