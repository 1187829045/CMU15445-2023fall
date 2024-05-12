//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// seq_scan_executor.cpp
//
// Identification: src/execution/seq_scan_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/seq_scan_executor.h"

namespace bustub {

SeqScanExecutor::SeqScanExecutor(ExecutorContext *exec_ctx, const SeqScanPlanNode *plan)
    : AbstractExecutor(exec_ctx), plan_(plan) {}
//初始化顺序扫描
void SeqScanExecutor::Init() {
  table_info_ = GetExecutorContext()->GetCatalog()->GetTable(plan_->GetTableOid());
  table_heap_ = table_info_->table_.get();
  auto iter = table_heap_->MakeIterator();
  rids_.clear();
  while (!iter.IsEnd()) {
    rids_.push_back(iter.GetRID());
    ++iter;
  }
  rids_iter_ = rids_.begin();
}
auto SeqScanExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  TupleMeta meta{};
  do {
    if (rids_iter_ == rids_.end()) {
      return false;
    }
    meta = table_heap_->GetTupleMeta(*rids_iter_);
    if (!meta.is_deleted_) {
      *tuple = table_heap_->GetTuple(*rids_iter_).second;
      *rid = *rids_iter_;
    }
    ++rids_iter_;
  } while (meta.is_deleted_ || (plan_->filter_predicate_ != nullptr &&
                                !plan_->filter_predicate_->Evaluate(tuple, table_info_->schema_).GetAs<bool>()));
  return true;
}
}  // namespace bustub
