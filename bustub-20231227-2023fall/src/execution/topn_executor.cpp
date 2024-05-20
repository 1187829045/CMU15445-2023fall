#include "execution/executors/topn_executor.h"

namespace bustub {
TopNExecutor::TopNExecutor(ExecutorContext *exec_ctx, const TopNPlanNode *plan,
                           std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx){
  this->plan_ = plan;
  this->child_executor_ = std::move(child_executor);
}

void TopNExecutor::Init() {
  child_executor_->Init();
  //使用优先队列存储topN，升序用大顶堆，降序用小顶堆
  std::priority_queue<Tuple, std::vector<Tuple>, HeapComparator> heap(
      HeapComparator(&this->GetOutputSchema(), plan_->GetOrderBy()));
  Tuple tuple{};
  RID rid{};
  //遍历子执行器，将子执行器返回的元组加入优先队列
  while (child_executor_->Next(&tuple, &rid)) {
    heap.push(tuple);
    heap_size_++;
    //因為只需要topN个元组，所以当优先队列大小大于topN时，弹出堆顶元组（如果是升序，堆顶是最大的元组，如果是降序，堆顶是最小的元组）
    if (heap.size() > plan_->GetN()) {
      heap.pop();
      heap_size_--;
    }
  }
  while (!heap.empty()) {
    this->top_entries_.push(heap.top());
    heap.pop();
  }
}

auto TopNExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  if (top_entries_.empty()) {
    return false;
  }
  *tuple = top_entries_.top();
  top_entries_.pop();
  return true;
}
auto TopNExecutor::GetNumInHeap() -> size_t {
     return top_entries_.size();
}
}  // namespace bustub
