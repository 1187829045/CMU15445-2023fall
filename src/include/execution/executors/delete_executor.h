#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "execution/executor_context.h"
#include "execution/executors/abstract_executor.h"
#include "execution/plans/delete_plan.h"
#include "storage/table/tuple.h"

namespace bustub {

/**
 * DeletedExecutor 对表执行删除。
 * 删除的值总是从子项中提取。
 */
class DeleteExecutor : public AbstractExecutor {
 public:
  /**
   * 构造一个新的DeleteExecutor实例。
   * @param exec_ctx 执行器上下文
   * @param plan 要执行的删除计划
   * @param child_executor 提供删除的子执行器
   */
  DeleteExecutor(ExecutorContext *exec_ctx, const DeletePlanNode *plan,
                 std::unique_ptr<AbstractExecutor> &&child_executor);

  /** Initialize the delete */
  void Init() override;

  /**
   * 产生从表中删除的行数。
   * @param[out] tuple 表示从表中删除的行数的整数元组
   * @param[out] rid 删除产生的下一个元组RID（忽略，未使用）
   * 如果生成了元组则返回`true`，如果没有更多的元组则返回`false`
   *
   * 注意：DeleteExecutor::Next() 不使用 `rid` 输出参数。
   * 注意：DeleteExecutor::Next() 返回 true，仅生成一次删除的行数。
   */
  auto Next([[maybe_unused]] Tuple *tuple, RID *rid) -> bool override;

  /** @return 删除的输出模式 */
  auto GetOutputSchema() const -> const Schema & override { return plan_->OutputSchema(); };

 private:
  /** 要执行的删除计划节点 */
  const DeletePlanNode *plan_;

  /** 子执行器，从中提取已删除元组的 RID */
  std::unique_ptr<AbstractExecutor> child_executor_;

  /** Whether the Next() has been called before */
  bool called_;
};
}  // namespace bustub
