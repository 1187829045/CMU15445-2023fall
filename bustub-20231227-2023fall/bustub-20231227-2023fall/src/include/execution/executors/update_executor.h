#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "execution/executor_context.h"
#include "execution/executors/abstract_executor.h"
#include "execution/plans/update_plan.h"
#include "storage/table/tuple.h"
#include "type/value_factory.h"

namespace bustub {

/**
 * UpdateExecutor 对表执行更新。
 * 更新的值总是从子项中提取。
 */
class UpdateExecutor : public AbstractExecutor {
  friend class UpdatePlanNode;

 public:
  /**
   * 构造一个新的UpdateExecutor实例。
   * @param exec_ctx 执行器上下文
   * @param plan 要执行的更新计划
   * @param child_executor 提供更新的子执行器
   */
  UpdateExecutor(ExecutorContext *exec_ctx, const UpdatePlanNode *plan,
                 std::unique_ptr<AbstractExecutor> &&child_executor);

  /** Initialize the update */
  void Init() override;

  /**
   * 从更新中产生下一个元组。
   * @param[out] tuple 更新产生的下一个元组
   * @param[out] rid 更新产生的下一个元组 RID（忽略此）
   * 如果生成了元组则返回`true`，如果没有更多的元组则返回`false`
   *
   * 注意：UpdateExecutor::Next() 不使用 `rid` 输出参数。
   */
  auto Next([[maybe_unused]] Tuple *tuple, RID *rid) -> bool override;

  /** @return 更新的输出模式 */
  auto GetOutputSchema() const -> const Schema & override { return plan_->OutputSchema(); }

 private:
  /** 要执行的更新计划节点 */
  const UpdatePlanNode *plan_;

  /** 标识应该更新的表的元数据 */
  const TableInfo *table_info_;
  
  /** 从中获取值的子执行器 */
  std::unique_ptr<AbstractExecutor> child_executor_;
  bool is_end_;
};
}  // namespace bustub
