
//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// aggregation_executor.h
//
// Identification: src/include/execution/executors/aggregation_executor.h
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "common/util/hash_util.h"
#include "container/hash/hash_function.h"
#include "execution/executor_context.h"
#include "execution/executors/abstract_executor.h"
#include "execution/expressions/abstract_expression.h"
#include "execution/plans/aggregation_plan.h"
#include "storage/table/tuple.h"
#include "type/value_factory.h"

namespace bustub {

/**
 * 一个简化的哈希表，具有聚合所需的所有功能。
 */
class SimpleAggregationHashTable {
 public:
  /**
   * 构造一个新的 SimpleAggregationHashTable 实例。
   * @param agg_exprs 聚合表达式
   * @param agg_types 聚合类型
   */
  SimpleAggregationHashTable(const std::vector<AbstractExpressionRef> &agg_exprs,
                             const std::vector<AggregationType> &agg_types)
      : agg_exprs_{agg_exprs}, agg_types_{agg_types} {}

  /** @return 此聚合执行器的初始聚合值 */
  auto GenerateInitialAggregateValue() -> AggregateValue {
    std::vector<Value> values{};
    for (const auto &agg_type : agg_types_) {
      switch (agg_type) {
        case AggregationType::CountStarAggregate:  // 对于 COUNT(*) 聚合，初始计数值为零
          values.emplace_back(ValueFactory::GetIntegerValue(0));
          break;
        case AggregationType::CountAggregate:  //计数聚合
        case AggregationType::SumAggregate:
        case AggregationType::MinAggregate:
        case AggregationType::MaxAggregate:
          // 其他从 null 开始。
          values.emplace_back(ValueFactory::GetNullValueByType(TypeId::INTEGER));
          break;
      }
    }
    return {values};
  }
  /**
   * 待办事项（学生）
   *
   * 将输入合并到聚合结果中。
   * @param[out] result 输出聚合值
   * @param input 输入值
   */
  void CombineAggregateValues(AggregateValue *result, const AggregateValue &input) {
    for (uint32_t i = 0; i < agg_exprs_.size(); i++) {
      switch (agg_types_[i]) {
        case AggregationType::CountStarAggregate:
          result->aggregates_[i] = result->aggregates_[i].Add(ValueFactory::GetIntegerValue(1));
          break;
        case AggregationType::CountAggregate:
          if (result->aggregates_[i].IsNull()) {
            result->aggregates_[i] = ValueFactory::GetIntegerValue(0);
          }
          if (!input.aggregates_[i].IsNull()) {
            result->aggregates_[i] = result->aggregates_[i].Add(ValueFactory::GetIntegerValue(1));
          }
          break;
        case AggregationType::SumAggregate:
          if (result->aggregates_[i].IsNull()) {
            result->aggregates_[i] = input.aggregates_[i];
          } else if (!input.aggregates_[i].IsNull()) {
            result->aggregates_[i] = result->aggregates_[i].Add(input.aggregates_[i]);
          }
          break;
        case AggregationType::MinAggregate:
          if (result->aggregates_[i].IsNull()) {
            result->aggregates_[i] = input.aggregates_[i];
          } else if (!input.aggregates_[i].IsNull()) {
            result->aggregates_[i] = result->aggregates_[i].Min(input.aggregates_[i]);
          }
          break;
        case AggregationType::MaxAggregate:
          if (result->aggregates_[i].IsNull()) {
            result->aggregates_[i] = input.aggregates_[i];
          } else if (!input.aggregates_[i].IsNull()) {
            result->aggregates_[i] = result->aggregates_[i].Max(input.aggregates_[i]);
          }
          break;
      }
    }
  }
  /**
   * 将一个值插入到哈希表中，然后将其与当前聚合相结合。
   * @param agg_key 要插入的键
   * @param agg_val 要插入的值
   */
  void InsertCombine(const AggregateKey &agg_key, const AggregateValue &agg_val) {
    if (ht_.count(agg_key) == 0) {
      ht_.insert({agg_key, GenerateInitialAggregateValue()});
    }
    CombineAggregateValues(&ht_[agg_key], agg_val);
  }
  void InsertEmptyCombine() { ht_.insert({{std::vector<Value>()}, GenerateInitialAggregateValue()}); }
  /**
   * Clear the hash table
   */
  void Clear() { ht_.clear(); }

  /** 聚合哈希表上的迭代器 */
  class Iterator {
   public:
    /** 为聚合映射创建一个迭代器。 */
    explicit Iterator(std::unordered_map<AggregateKey, AggregateValue>::const_iterator iter) : iter_{iter} {}

    /** @return 迭代器的键 */
    auto Key() -> const AggregateKey & { return iter_->first; }

    /** @return 迭代器的值 */
    auto Val() -> const AggregateValue & { return iter_->second; }

    /** @return 递增之前的迭代器 */
    auto operator++() -> Iterator & {
      ++iter_;
      return *this;
    }

    /** @return `true` if both iterators are identical完全相同的 */
    auto operator==(const Iterator &other) -> bool { return this->iter_ == other.iter_; }

    /** @return `true` if both iterators are different */
    auto operator!=(const Iterator &other) -> bool { return this->iter_ != other.iter_; }

   private:
    /** Aggregates map */
    std::unordered_map<AggregateKey, AggregateValue>::const_iterator iter_;
  };

  /** @return Iterator to the start of the hash table */
  auto Begin() -> Iterator { return Iterator{ht_.cbegin()}; }

  /** @return Iterator to the end of the hash table */
  auto End() -> Iterator { return Iterator{ht_.cend()}; }

  auto Size() -> size_t { return ht_.size(); }

 private:
  //这个哈希表做聚合键到聚合值的映射
  std::unordered_map<AggregateKey, AggregateValue> ht_{};
  /** 我们拥有的聚合表达式 */
  const std::vector<AbstractExpressionRef> &agg_exprs_;
  /** 我们拥有的聚合类型 */
  const std::vector<AggregationType> &agg_types_;
};

/**
 * AggregationExecutor 执行聚合操作（例如 COUNT、SUM、MIN、MAX）
 * 在子执行器生成的元组上。
 */
class AggregationExecutor : public AbstractExecutor {
 public:
  /**
   * 构造一个新的AggregationExecutor实例。
   * @param exec_ctx 执行器上下文
   * @param plan 要执行的插入计划
   * @param child_executor 从中提取插入元组的子执行器（可能是“nullptr”）
   * 从中拉出插入元组的子执行器（可能是“nullptr”）
   */
  AggregationExecutor(ExecutorContext *exec_ctx, const AggregationPlanNode *plan,
                      std::unique_ptr<AbstractExecutor> &&child_executor);

  /** Initialize the aggregation */
  void Init() override;

  /**
   * 从插入中产生下一个元组。
   * @param[out] tuple 聚合产生的下一个元组
   * @param[out] rid 聚合产生的下一个元组RID
   * 如果生成了元组则返回`true`，如果没有更多的元组则返回`false`
   */
  auto Next(Tuple *tuple, RID *rid) -> bool override;

  /** @return The output schema for the aggregation */
  /** @return 聚合的输出模式 */
  auto GetOutputSchema() const -> const Schema & override { return plan_->OutputSchema(); };

  /** Do not use or remove this function, otherwise you will get zero points. */
  auto GetChildExecutor() const -> const AbstractExecutor *;

 private:
  /** @return The tuple as an AggregateKey */
  /** @return 作为 AggregateKey 的元组 */
  auto MakeAggregateKey(const Tuple *tuple) -> AggregateKey {
    std::vector<Value> keys;
    for (const auto &expr : plan_->GetGroupBys()) {
      keys.emplace_back(expr->Evaluate(tuple, child_executor_->GetOutputSchema()));
    }
    return {keys};
  }

  /** @return The tuple as an AggregateValue */
  /** @return 作为 AggregateValue 的元组 */
  auto MakeAggregateValue(const Tuple *tuple) -> AggregateValue {
    std::vector<Value> vals;
    for (const auto &expr : plan_->GetAggregates()) {
      vals.emplace_back(expr->Evaluate(tuple, child_executor_->GetOutputSchema()));
    }
    return {vals};
  }

 private:
  /** The aggregation plan node */
  const AggregationPlanNode *plan_;

  /** The child executor that produces tuples over which the aggregation is computed */
  //生成用于计算聚合的元组的子执行器
  std::unique_ptr<AbstractExecutor> child_executor_;

  /** 简单聚合哈希表 */
  SimpleAggregationHashTable aht_;

  /** 简单聚合哈希表迭代器 */
  SimpleAggregationHashTable::Iterator aht_iterator_;
};
}  // namespace bustub