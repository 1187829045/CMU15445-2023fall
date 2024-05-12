#pragma once

#include <vector>
/*ExecutorContext是在数据库系统中用于执行查询计划的上下文环境。
它存储了执行查询所需的所有上下文信息和资源，以便执行器（Executor）能够正确地执行查询计划。*/
#include "execution/executor_context.h"
// AbstractExecutor 实现了 Volcano 一次元组迭代器模型。
//这是BustTub中所有执行器执行的基类
//引擎继承，并定义了所有执行器支持的最小接口。
#include "execution/executors/abstract_executor.h"
#include "execution/plans/seq_scan_plan.h"
#include "storage/table/tuple.h"

namespace bustub {

/**
 * SeqScanExecutor 执行器 执行顺序表扫描。
 */
class SeqScanExecutor : public AbstractExecutor {
 public:
  // 构造一个新的 SeqScanExecutor 实例。
  //@param exec_ctx 执行器上下文
  //@param plan 要执行的顺序扫描计划
  SeqScanExecutor(ExecutorContext *exec_ctx, const SeqScanPlanNode *plan);

  //初始化顺序扫描

  void Init() override;

  // 从顺序扫描中产生下一个元组。
  // @param[out] tuple 扫描产生的下一个元组
  // @param[out] rid 扫描产生的下一个元组 RID
  // 如果生成了元组则返回`true`，如果没有更多的元组则返回`false`
  auto Next(Tuple *tuple, RID *rid) -> bool override;

  /** @return 顺序扫描的输出模式 */
  auto GetOutputSchema() const -> const Schema & override { return plan_->OutputSchema(); }

 private:
  /** 要执行的顺序扫描计划节点 */
  const SeqScanPlanNode *plan_;
  TableInfo *table_info_;
  TableHeap *table_heap_;  //表堆是表示物理表在磁盘上的存储结构
  std::vector<RID> rids_;
  std::vector<RID>::iterator rids_iter_;
};
}  // namespace bustub
