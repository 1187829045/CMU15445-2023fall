#include <algorithm>
#include <memory>
#include "catalog/column.h"
#include "catalog/schema.h"
#include "common/exception.h"
#include "common/macros.h"
#include "execution/expressions/column_value_expression.h"
#include "execution/expressions/comparison_expression.h"
#include "execution/expressions/constant_value_expression.h"
#include "execution/expressions/logic_expression.h"
#include "execution/plans/abstract_plan.h"
#include "execution/plans/filter_plan.h"
#include "execution/plans/hash_join_plan.h"
#include "execution/plans/nested_loop_join_plan.h"
#include "execution/plans/projection_plan.h"
#include "optimizer/optimizer.h"
#include "type/type_id.h"

namespace bustub {
//这段代码的主要功能是解析一个包含多个条件的连接谓词，并提取出用于哈希连接的左右连接键表达式。
//它通过递归处理逻辑表达式（如 AND）和识别比较表达式（如
//=）来实现这一点，从而为优化器提供必要的信息以转换嵌套循环连接为哈希连接。
// 解析一个逻辑表达式，并提取出左右两侧的关键表达式
void ParseAndExpression(const AbstractExpressionRef &predicate,
                        std::vector<AbstractExpressionRef> *left_key_expressions,
                        std::vector<AbstractExpressionRef> *right_key_expressions) {
  // 尝试将谓词转换为逻辑表达式，与或非
  auto *logic_expression_ptr = dynamic_cast<LogicExpression *>(predicate.get());
  // 递归处理逻辑逻辑表达式
  if (logic_expression_ptr != nullptr) {
    // left child
    ParseAndExpression(logic_expression_ptr->GetChildAt(0), left_key_expressions, right_key_expressions);
    // right child
    ParseAndExpression(logic_expression_ptr->GetChildAt(1), left_key_expressions, right_key_expressions);
  }
  // 尝试将谓词转换为比较表达式
  auto *comparison_ptr = dynamic_cast<ComparisonExpression *>(predicate.get());
  // 如果是比较表达式
  if (comparison_ptr != nullptr) {
    auto column_value_1 = dynamic_cast<const ColumnValueExpression &>(*comparison_ptr->GetChildAt(0));
    // auto column_value_2 = dynamic_cast<const ColumnValueExpression &>(*comparison_ptr->GetChildAt(1));
    // 区分每个数据元素是从左侧表还是右侧表提取的，例如 A.id = B.id时，系统需要知道 A.id 和 B.id 分别属于哪个数据源
    if (column_value_1.GetTupleIdx() == 0) {
      left_key_expressions->emplace_back(comparison_ptr->GetChildAt(0));
      right_key_expressions->emplace_back(comparison_ptr->GetChildAt(1));
    } else {
      left_key_expressions->emplace_back(comparison_ptr->GetChildAt(1));
      right_key_expressions->emplace_back(comparison_ptr->GetChildAt(0));
    }
  }
}
// TODO(student): implement NestedLoopJoin -> HashJoin optimizer rule
// 2023 年秋季注意事项：您应该支持任意数量的等值条件连接键的合取（conjunction）：
// 例如：<column expr> = <column expr> AND <column expr> = <column expr> AND ...
auto Optimizer::OptimizeNLJAsHashJoin(const AbstractPlanNodeRef &plan) -> AbstractPlanNodeRef {
  std::vector<AbstractPlanNodeRef> optimized_children;
  for (const auto &child : plan->GetChildren()) {
    // 递归调用
    optimized_children.emplace_back(OptimizeNLJAsHashJoin(child));
  }
  auto optimized_plan = plan->CloneWithChildren(std::move(optimized_children));
  if (optimized_plan->GetType() == PlanType::NestedLoopJoin) {
    const auto &join_plan = dynamic_cast<const NestedLoopJoinPlanNode &>(*optimized_plan);
    // 获取谓词
    auto predicate = join_plan.Predicate();
    std::vector<AbstractExpressionRef> left_key_expressions;
    std::vector<AbstractExpressionRef> right_key_expressions;
    // 提取左右两侧关键表达式，分别放到left_key_expressions和right_key_expressions里)
    ParseAndExpression(predicate, &left_key_expressions, &right_key_expressions);
    return std::make_shared<HashJoinPlanNode>(join_plan.output_schema_, join_plan.GetLeftPlan(),
                                              join_plan.GetRightPlan(), left_key_expressions, right_key_expressions,
                                              join_plan.GetJoinType());
  }
  return optimized_plan;
}

}  // namespace bustub
