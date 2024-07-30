/*
可以使用DELETE语句来规划DeletePlanNode。
它只有一个子项，其中包含要从表中删除的记录。
您的删除执行器应该生成一个整数输出，表示它从表中删除的行数。它还需要更新所有受影响的索引。
 您可以假设DeleteExecutor 始终位于它出现的查询计划的根部。 DeleteExecutor 不应修改其结果集。
提示：要删除一个元组，您需要从子执行器获取一个 RID，并更新该元组相应 TupleMeta 的 is_deleted_ 字段。
 */
#include <memory>

#include "execution/executors/delete_executor.h"

namespace bustub {

DeleteExecutor::DeleteExecutor(ExecutorContext *exec_ctx, const DeletePlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), child_executor_(std::move(child_executor)) {}

void DeleteExecutor::Init() {
  child_executor_->Init();
  called_ = false;
}
// tuple 表示从表中删除的行数的整数元组
auto DeleteExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) -> bool {
  TableInfo *table_info = GetExecutorContext()->GetCatalog()->GetTable(plan_->GetTableOid());
  Transaction *tx = GetExecutorContext()->GetTransaction();
  TableHeap *table_heap = table_info->table_.get();
  std::vector<IndexInfo *> index_info_vector = GetExecutorContext()->GetCatalog()->GetTableIndexes(table_info->name_);

  Tuple child_tuple{};
  int size = 0;
  while (child_executor_->Next(&child_tuple, rid)) {
    table_heap->UpdateTupleMeta(TupleMeta{tx->GetTransactionTempTs(), true}, *rid);
    for (auto &index_info : index_info_vector) {
      index_info->index_->DeleteEntry(
          child_tuple.KeyFromTuple(table_info->schema_, index_info->key_schema_, index_info->index_->GetKeyAttrs()),
          *rid, tx);
    }

    size++;
  }

  std::vector<Value> values{};
  values.reserve(GetOutputSchema().GetColumnCount());
  values.emplace_back(INTEGER, size);

  *tuple = Tuple{values, &GetOutputSchema()};

  if (size == 0 && !called_) {
    called_ = true;
    return true;
  }

  called_ = true;
  return size != 0;
}
}  // namespace bustub
