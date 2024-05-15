

/*
可以使用 UPDATE 语句来规划 UpdatePlanNode。
 它只有一个子项，其中包含要在表中更新的记录。
UpdateExecutor 修改指定表中的现有元组。
执行器将生成一个整数类型的元组作为输出，指示已更新了多少行。
 请记住更新受更新影响的所有索引。

提示：要实现更新，首先删除受影响的元组，然后插入新元组。
 除非您正在为项目 4 实现排行榜优化，否则请勿使用 TableHeap UpdateTupleInplaceUnsafe 函数。
 */
#include <memory>

#include "execution/executors/update_executor.h"

namespace bustub {

UpdateExecutor::UpdateExecutor(ExecutorContext *exec_ctx, const UpdatePlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_{plan}, child_executor_{std::move(child_executor)} {
  // 截至 2022 年秋季，您无需实现更新执行器即可在项目 3 / 项目 4 中获得满分。
}

void UpdateExecutor::Init() {
  child_executor_->Init();
  table_info_ = exec_ctx_->GetCatalog()->GetTable(plan_->GetTableOid());
  is_end_ = false;
}

auto UpdateExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) -> bool {
  if (is_end_) {
    return false;
  }
  Transaction *tx = GetExecutorContext()->GetTransaction();
  TableInfo *table_info = GetExecutorContext()->GetCatalog()->GetTable(plan_->GetTableOid());
  std::vector<IndexInfo *> index_info_vector = GetExecutorContext()->GetCatalog()->GetTableIndexes(table_info->name_);
  Tuple old_tuple{};
  int32_t update_count = 0;

  while (child_executor_->Next(&old_tuple, rid)) {
    std::vector<Value> values{};
    values.reserve(child_executor_->GetOutputSchema().GetColumnCount());
    for (const auto &expr : plan_->target_expressions_) {
      values.push_back(expr->Evaluate(&old_tuple, child_executor_->GetOutputSchema()));
    }

    auto to_update_tuple = Tuple{values, &child_executor_->GetOutputSchema()};

    bool updated =
        table_info_->table_->UpdateTupleInPlace(TupleMeta{tx->GetTransactionTempTs(), false}, to_update_tuple, *rid);

    if (updated) {
      update_count++;
      for (auto &index_info : index_info_vector) {
        index_info->index_->DeleteEntry(
            old_tuple.KeyFromTuple(table_info->schema_, index_info->key_schema_, index_info->index_->GetKeyAttrs()),
            *rid, tx);
        index_info->index_->InsertEntry(to_update_tuple.KeyFromTuple(table_info->schema_, index_info->key_schema_,
                                                                     index_info->index_->GetKeyAttrs()),
                                        *rid, tx);
      }
    }
  }
  std::vector<Value> values{};
  values.reserve(GetOutputSchema().GetColumnCount());
  values.emplace_back(TypeId::INTEGER, update_count);
  *tuple = Tuple{values, &GetOutputSchema()};
  is_end_ = true;
  return true;
}

}  // namespace bustub
