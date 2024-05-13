//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// index_scan_executor.cpp
//
// Identification: src/execution/index_scan_executor.cpp
//
// Copyright (c) 2015-19, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include "execution/executors/index_scan_executor.h"

namespace bustub {
IndexScanExecutor::IndexScanExecutor(ExecutorContext *exec_ctx, const IndexScanPlanNode *plan)
    : AbstractExecutor(exec_ctx),plan_(plan){}

void IndexScanExecutor::Init() {
  IndexInfo *index_info = GetExecutorContext()->GetCatalog()->GetIndex(plan_->index_oid_);
  auto *hash_index = dynamic_cast<HashTableIndexForTwoIntegerColumn *>(index_info->index_.get());
  rids_.clear();
  if(plan_->filter_predicate_!=nullptr){
    const auto right_exptr=dynamic_cast<ConstantValueExpression *>(plan_->filter_predicate_->children_[1].get());
    Value v = right_exptr->val_;
    hash_index->ScanKey(Tuple{{v}, index_info->index_->GetKeySchema()}, &rids_, GetExecutorContext()->GetTransaction());
  }
   rids_iter_=rids_.begin();
}

auto IndexScanExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  IndexInfo *index_info = GetExecutorContext()->GetCatalog()->GetIndex(plan_->GetIndexOid());
  TableInfo *table_info = GetExecutorContext()->GetCatalog()->GetTable(index_info->table_name_);

  TupleMeta meta{};
  do {
   if(rids_iter_==rids_.end()){
    return false;
   }
   *rid=*rids_iter_;
    meta=table_info->table_->GetTupleMeta(*rid);
   *tuple=table_info->table_->GetTuple(*rid).second;
    rids_iter_++;
  }while(meta.is_deleted_);

  return true;
}

}  // namespace bustub
