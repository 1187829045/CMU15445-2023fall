#include "execution/executors/sort_executor.h"

namespace bustub {

SortExecutor::SortExecutor(ExecutorContext *exec_ctx, const SortPlanNode *plan,
                           std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx) {
  this->plan_ = plan;
  this->child_executor_ = std::move(child_executor);
}
void SortExecutor::Init() {
  child_executor_->Init();
  Tuple tuple{};
  RID rid{};
  while (child_executor_->Next(&tuple, &rid)) {
    tuples_.emplace_back(tuple);
  }
  // 获取排序字段
  auto order_by = plan_->GetOrderBy();
  // 排序
  std::sort(tuples_.begin(), tuples_.end(), Comparator(&this->GetOutputSchema(), order_by));
  iter_ = tuples_.begin();
}

auto SortExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  // 调用的时候返回，从头到尾一个个返回
  if (iter_ != tuples_.end()) {
    *tuple = *iter_;
    ++iter_;
    return true;
  }
  return false;
}
}  // namespace bustub
