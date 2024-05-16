//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// nested_loop_join_executor.cpp
//
// Identification: src/execution/nested_loop_join_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/nested_loop_join_executor.h"
#include "binder/table_ref/bound_join_ref.h"
#include "common/exception.h"
#include "type/value_factory.h"

namespace bustub {

NestedLoopJoinExecutor::NestedLoopJoinExecutor(ExecutorContext *exec_ctx, const NestedLoopJoinPlanNode *plan,
                                               std::unique_ptr<AbstractExecutor> &&left_executor,
                                               std::unique_ptr<AbstractExecutor> &&right_executor)
    : AbstractExecutor(exec_ctx),
      plan_(plan),
      left_executor_(std::move(left_executor)),
      right_executor_(std::move(right_executor)) {
  if (!(plan->GetJoinType() == JoinType::LEFT || plan->GetJoinType() == JoinType::INNER)) {
    // Note for 2023 Fall: You ONLY need to implement left join and inner join.
    throw bustub::NotImplementedException(fmt::format("join type {} not supported", plan->GetJoinType()));
  }
}

auto NestedLoopJoinExecutor::LeftAntiJoinTuple(Tuple *left_tuple) -> Tuple {  //左连接
  std::vector<Value> values{};
  values.reserve(GetOutputSchema().GetColumnCount());
  for (uint32_t idx = 0; idx < left_executor_->GetOutputSchema().GetColumnCount(); idx++) {
    values.push_back(left_tuple->GetValue(&left_executor_->GetOutputSchema(), idx));
  }
  //对于左连接中不存在的列，添加 NULL 值
  for (uint32_t idx = 0; idx < right_executor_->GetOutputSchema().GetColumnCount(); idx++) {
    // 获取右表元组中的值，但由于左连接中不存在右表的元组，因此添加 NULL 值
    values.push_back(ValueFactory::GetNullValueByType(plan_->GetRightPlan()->OutputSchema().GetColumn(idx).GetType()));
  }
  return Tuple{values, &GetOutputSchema()};
}

auto NestedLoopJoinExecutor::InnerJoinTuple(Tuple *left_tuple, Tuple *right_tuple) -> Tuple {  //内连接
  std::vector<Value> values{};
  values.reserve(GetOutputSchema().GetColumnCount());

  for (uint32_t idx = 0; idx < left_executor_->GetOutputSchema().GetColumnCount(); idx++) {
    values.push_back(left_tuple->GetValue(&left_executor_->GetOutputSchema(), idx));
  }
  for (uint32_t idx = 0; idx < right_executor_->GetOutputSchema().GetColumnCount(); idx++) {
    values.push_back(right_tuple->GetValue(&right_executor_->GetOutputSchema(), idx));
  }

  return Tuple{values, &GetOutputSchema()};
}

void NestedLoopJoinExecutor::Init() {
  left_executor_->Init();
  right_executor_->Init();
  RID left_rid;
  left_ret_ = left_executor_->Next(&left_tuple_, &left_rid);
  left_done_ = false;
}

auto NestedLoopJoinExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  Tuple right_tuple{};  // 用于存储右表中的元组
  RID left_rid;         // 左表中当前元组的 RID
  RID right_rid;        // 右表中当前元组的 RID

  while (true) {       // 进入循环，直到返回结果或者遍历结束
    if (!left_ret_) {  // 如果左表中的元组已经遍历完毕
      return false;    // 返回 false，表示没有更多的结果了
    }
    if (!right_executor_->Next(&right_tuple, &right_rid)) {  // 如果右表中的元组已经遍历完毕
      if (plan_->GetJoinType() == JoinType::LEFT &&
          !left_done_) {  // 如果是左连接且左表中的当前元组还没有匹配到右表的任何元组
        *tuple = LeftAntiJoinTuple(&left_tuple_);  // 生成左连接结果中不存在右表元组的元组
        *rid = tuple->GetRid();                    // 获取元组的 RID

        left_done_ = true;  // 标记左表中的当前元组已经处理过
        return true;        // 返回 true，表示找到了一个结果
      }

      right_executor_->Init();  // 重新初始化右表的扫描，以便重新开始扫描右表
      left_ret_ = left_executor_->Next(&left_tuple_, &left_rid);  // 获取下一个左表中的元组
      left_done_ = false;                                         // 重置 left_done_ 标志
      continue;                                                   // 继续循环，尝试下一个左表中的元组
    }

    auto ret = plan_->Predicate()->EvaluateJoin(
        &left_tuple_, left_executor_->GetOutputSchema(), &right_tuple,
        right_executor_->GetOutputSchema());   // 利用 JOIN 条件判断左右表中的元组是否满足条件
    if (!ret.IsNull() && ret.GetAs<bool>()) {  // 如果左右表中的元组满足 JOIN 条件
      *tuple = InnerJoinTuple(&left_tuple_, &right_tuple);  // 生成内连接结果的元组
      *rid = tuple->GetRid();                               // 获取元组的 RID

      left_done_ = true;  // 标记左表中的当前元组已经处理过
      return true;        // 返回 true，表示找到了一个结果
    }
  }
}

}  // namespace bustub
