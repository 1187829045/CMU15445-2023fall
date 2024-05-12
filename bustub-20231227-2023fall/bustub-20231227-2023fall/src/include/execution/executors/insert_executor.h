#pragma once

#include <memory>
#include <utility>

#include "execution/executor_context.h"
#include "execution/executors/abstract_executor.h"
#include "execution/plans/insert_plan.h"
#include "storage/table/tuple.h"

namespace bustub {

/**
 * InsertExecutor 执行表上的插入操作。
 * 插入的值始终从子执行器中获取。
 */
class InsertExecutor : public AbstractExecutor {
 public:
  /**
   * 构造一个新的 InsertExecutor 实例。
   * @param exec_ctx 执行器上下文
   * @param plan 要执行的插入计划
   * @param child_executor 从中获取插入元组的子执行器
   */
  InsertExecutor(ExecutorContext *exec_ctx, const InsertPlanNode *plan,
                 std::unique_ptr<AbstractExecutor> &&child_executor);

  /** 初始化插入 */
  void Init() override;

  /**
   * 返回插入到表中的行数。
   * @param[out] tuple 整数元组，指示插入到表中的行数
   * @param[out] rid 插入生成的下一个元组 RID（忽略，未使用）
   * @return 如果产生了元组，则为`true`，如果没有更多的元组，则为`false`
   *
   * 注意：InsertExecutor::Next() 不使用`rid`输出参数。
   * 注意：InsertExecutor::Next() 仅在产生插入行数的情况下返回 true。
   */
  auto Next([[maybe_unused]] Tuple *tuple, RID *rid) -> bool override;

  /** @return 插入的输出模式 */
  auto GetOutputSchema() const -> const Schema & override { return plan_->OutputSchema(); };

 private:
  /** 要执行的插入计划节点 */
  const InsertPlanNode *plan_;
  // 从中获取元组的子执行器
  std::unique_ptr<AbstractExecutor> child_executor_;
  /** Next()之前是否被调用过 */
  bool called_;
};

}  // namespace bustub
