//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// nested_loop_join_executor.h
//
// Identification: src/include/execution/executors/nested_loop_join_executor.h
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <utility>

#include "execution/executor_context.h"
#include "execution/executors/abstract_executor.h"
#include "execution/plans/nested_loop_join_plan.h"
#include "storage/table/tuple.h"

namespace bustub {

/**
 * NestedLoopJoinExecutor执行两个表上的嵌套循环JOIN。
 */
class NestedLoopJoinExecutor : public AbstractExecutor {
 public:
  /**
   * 构造一个新的NestedLoopJoinExecutor实例。
   * @param exec_ctx 执行器上下文
   * @param plan 要执行的嵌套循环连接计划
   * @param left_executor 产生左连接侧元组的子执行器
   * @param right_executor 产生右连接侧元组的子执行器
   */
  NestedLoopJoinExecutor(ExecutorContext *exec_ctx, const NestedLoopJoinPlanNode *plan,
                         std::unique_ptr<AbstractExecutor> &&left_executor,
                         std::unique_ptr<AbstractExecutor> &&right_executor);

  /** 初始化连接 */
  void Init() override;

  auto LeftAntiJoinTuple(Tuple *left_tuple) -> Tuple;

  auto InnerJoinTuple(Tuple *left_tuple, Tuple *right_tuple) -> Tuple;
  /**
   * 从连接中产生下一个元组。
   * @param[out] tuple 连接生成的下一个元组
   * @param[out] rid 下一个元组的RID，嵌套循环连接不使用。
   * @return 如果产生了元组，则为“true”，如果没有更多元组，则为“false”。
   */
  auto Next(Tuple *tuple, RID *rid) -> bool override;

  /** @return 插入的输出模式 */
  auto GetOutputSchema() const -> const Schema & override { return plan_->OutputSchema(); };

 private:
  /** 要执行的NestedLoopJoin计划节点。 */
  const NestedLoopJoinPlanNode *plan_;

  std::unique_ptr<AbstractExecutor> left_executor_;
  /** The right child executor */
  std::unique_ptr<AbstractExecutor> right_executor_;
  /** 左孩子的 Next() 结果 */
  bool left_ret_;
  /** 从左孩子获取元组 */
  Tuple left_tuple_;
  /** 该左元组是否已与任何右元组匹配 */
  bool left_done_;
};

}  // namespace bustub
