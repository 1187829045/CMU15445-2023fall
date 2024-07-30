#pragma once

#include <memory>
#include <stack>
#include <utility>
#include <vector>
#include "execution/executor_context.h"
#include "execution/executors/abstract_executor.h"
#include "execution/plans/seq_scan_plan.h"
#include "execution/plans/topn_plan.h"
#include "storage/table/tuple.h"

namespace bustub {

// 用于排序的比较器
class HeapComparator {
 public:
  HeapComparator() { schema_ = nullptr; }
  HeapComparator(const Schema *schema, std::vector<std::pair<OrderByType, AbstractExpressionRef>> order_bys)
      : schema_(schema), order_bys_(std::move(order_bys)) {}

  auto operator()(const Tuple &t1, const Tuple &t2) -> bool {
    for (auto const &order_by : this->order_bys_) {
      const auto order_type = order_by.first;
      // 使用Evaluate获取值
      AbstractExpressionRef expr = order_by.second;
      Value v1 = expr->Evaluate(&t1, *schema_);
      Value v2 = expr->Evaluate(&t2, *schema_);
      if (v1.CompareEquals(v2) == CmpBool::CmpTrue) {
        continue;
      }
      // 如果是升序（ASC 或 DEFAULT），比较 v1 是否小于 v2（CompareLessThan）
      if (order_type == OrderByType::ASC || order_type == OrderByType::DEFAULT) {
        return v1.CompareLessThan(v2) == CmpBool::CmpTrue;
      }
      // 如果是降序（DESC），比较 v1 是否大于 v2（CompareGreaterThan）
      return v1.CompareGreaterThan(v2) == CmpBool::CmpTrue;
    }
    // 两个元组所有键都相等
    return false;
  }

 private:
  const Schema *schema_;
  // 两个参数：升序还是降序，用那个键的值
  std::vector<std::pair<OrderByType, AbstractExpressionRef>> order_bys_;
};
/**
 * TopNExecutor 执行器执行 TopN 操作。
 */
class TopNExecutor : public AbstractExecutor {
 public:
  /**
   * 构造一个新的 TopNExecutor 实例。
   * @param exec_ctx 执行器上下文
   * @param plan 要执行的 TopN 计划
   */
  TopNExecutor(ExecutorContext *exec_ctx, const TopNPlanNode *plan, std::unique_ptr<AbstractExecutor> &&child_executor);

  /** 初始化 TopN */
  void Init() override;

  /**
   * 从 TopN 中产生下一个元组。
   * @param[out] tuple TopN 产生的下一个元组
   * @param[out] rid TopN 产生的下一个元组 RID
   * @return 如果产生了元组，则返回 true，如果没有更多元组，则返回 false
   */
  auto Next(Tuple *tuple, RID *rid) -> bool override;

  /** @return TopN 的输出模式 */
  auto GetOutputSchema() const -> const Schema & override { return plan_->OutputSchema(); }

  /** 设置新的子执行器（仅用于测试） */
  void SetChildExecutor(std::unique_ptr<AbstractExecutor> &&child_executor) {
    child_executor_ = std::move(child_executor);
  }

  /** @return top_entries_ 容器的大小，该容器将在每个 child_executor->Next() 调用时调用。 */
  auto GetNumInHeap() -> size_t;

 private:
  /** The TopN plan node to be executed */
  const TopNPlanNode *plan_;
  /** The child executor from which tuples are obtained */
  std::unique_ptr<AbstractExecutor> child_executor_;
  // 按順序存储优先队列中的tuple
  /** The stack to store sorted top-n tuple*/
  std::stack<Tuple> top_entries_;
  size_t heap_size_{0};
};
}  // namespace bustub
