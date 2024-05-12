
//IndexScanExecutor 使用哈希索引进行点查找以检索元组的 RID。
//然后，操作员使用这些 RID 来检索相应表中的元组。 然后它一次发出一个这些元组。

//在本项目中，计划中索引对象的类型将始终为 HashTableIndexForTwoIntegerColumn。 
//您可以安全地转换它并将其存储在执行器对象中：
//htable_ = dynamic_cast<HashTableIndexForTwoIntegerColumn *>(index_info_->index_.get())
//然后，您可以使用哈希索引进行点查找，从表堆中查找元组，并根据谓词发出令人满意的元组。
//BusTub 仅支持具有单个唯一整数列的索引。
//我们的测试用例不会包含重复的键。 
//因此，点查找仅返回一个元组（如果存在）。
//您需要完成下一节中的优化器规则，将 SeqScan 转换为 IndexScan。

//提示：我们永远不会将重复的行插入到带有索引的表中。

//提示：如上所述，不要发出已删除的元组。
 
#pragma once

#include <vector>

#include "common/rid.h"
#include "execution/executor_context.h"
#include "execution/executors/abstract_executor.h"
#include "execution/plans/index_scan_plan.h"
#include "storage/table/tuple.h"

namespace bustub {
/**
  * IndexScanExecutor 对表执行索引扫描。
  */

class IndexScanExecutor : public AbstractExecutor {
 public:
  /**
    * 创建一个新的索引扫描执行器。
    * @param exec_ctx 执行器上下文
    * @param plan 要执行的索引扫描计划
    */
  IndexScanExecutor(ExecutorContext *exec_ctx, const IndexScanPlanNode *plan);

  auto GetOutputSchema() const -> const Schema & override { return plan_->OutputSchema(); }

  void Init() override;

  auto Next(Tuple *tuple, RID *rid) -> bool override;

 private:
 /** 要执行的索引扫描计划节点。 */
  const IndexScanPlanNode *plan_;
  std::vector<RID> rids_;
  std::vector<RID>::iterator rids_iter_;
};
}  // namespace bustub
