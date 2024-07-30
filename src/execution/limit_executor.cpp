//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// limit_executor.cpp
//
// Identification: src/execution/limit_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/limit_executor.h"

namespace bustub {

LimitExecutor::LimitExecutor(ExecutorContext *exec_ctx, const LimitPlanNode *plan,
                             std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx) {
  this->plan_ = plan;
  this->child_executor_ = std::move(child_executor);
}

void LimitExecutor::Init() {
  child_executor_->Init();
  std::size_t count = 0;
  auto limit = plan_->GetLimit();
  Tuple tuple{};
  RID rid{};
  // 获取符合条件数量的元组
  while (count < limit && child_executor_->Next(&tuple, &rid)) {
    count++;
    tuples_.emplace_back(tuple);
  }
  if (!tuples_.empty()) {
    iter_ = tuples_.begin();
  }
}

auto LimitExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  if (!tuples_.empty() && iter_ != tuples_.end()) {
    *tuple = *iter_;
    iter_++;
    return true;
  }
  return false;
}

}  // namespace bustub
