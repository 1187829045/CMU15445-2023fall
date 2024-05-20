#include "execution/plans/limit_plan.h"
#include "execution/plans/sort_plan.h"
#include "execution/plans/topn_plan.h"
#include "optimizer/optimizer.h"

namespace bustub {

auto Optimizer::OptimizeSortLimitAsTopN(const AbstractPlanNodeRef &plan) -> AbstractPlanNodeRef {
  // TODO(student): implement sort + limit -> top N optimizer rule
  // 对所有子节点递归应用这一优化
  std::vector<bustub::AbstractPlanNodeRef> optimized_children;
  for (const auto &child : plan->GetChildren()) {
    optimized_children.emplace_back(OptimizeSortLimitAsTopN(child));
  }
  auto optimized_plan = plan->CloneWithChildren(std::move(optimized_children));
  if (optimized_plan->GetType() == PlanType::Limit) {
    const auto &limit_plan = dynamic_cast<const LimitPlanNode &>(*optimized_plan);
    auto child = optimized_plan->children_[0];
    if (child->GetType() == PlanType::Sort) {
      const auto &sort_plan = dynamic_cast<const SortPlanNode &>(*child);
      return std::make_shared<TopNPlanNode>(optimized_plan->output_schema_, optimized_plan->children_[0],
                                            sort_plan.GetOrderBy(), limit_plan.limit_);
    }
  }
  return optimized_plan;
}
}  // namespace bustub
