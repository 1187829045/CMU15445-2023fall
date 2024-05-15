//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// aggregation_plan.h
//
// Identification: src/include/execution/plans/aggregation_plan.h
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "common/util/hash_util.h"
#include "execution/expressions/abstract_expression.h"
#include "execution/plans/abstract_plan.h"
#include "fmt/format.h"
#include "storage/table/tuple.h"

namespace bustub {

// 枚举类型，用于表示不同的聚合函数
enum class AggregationType { CountStarAggregate, CountAggregate, SumAggregate, MinAggregate, MaxAggregate };

// 聚合计划节点类，用于表示各种 SQL 聚合函数
class AggregationPlanNode : public AbstractPlanNode {
 public:
  // 构造函数，初始化聚合计划节点
  AggregationPlanNode(SchemaRef output_schema, AbstractPlanNodeRef child, std::vector<AbstractExpressionRef> group_bys,
                      std::vector<AbstractExpressionRef> aggregates, std::vector<AggregationType> agg_types)
      : AbstractPlanNode(std::move(output_schema), {std::move(child)}),
        group_bys_(std::move(group_bys)),
        aggregates_(std::move(aggregates)),
        agg_types_(std::move(agg_types)) {}

  // 返回计划节点类型
  auto GetType() const -> PlanType override { return PlanType::Aggregation; }

  // 返回聚合计划节点的子节点
  auto GetChildPlan() const -> AbstractPlanNodeRef {
    BUSTUB_ASSERT(GetChildren().size() == 1, "Aggregation expected to only have one child.");
    return GetChildAt(0);
  }

  // 返回指定索引处的 group by 表达式
  auto GetGroupByAt(uint32_t idx) const -> const AbstractExpressionRef & { return group_bys_[idx]; }

  // 返回 group by 表达式
  auto GetGroupBys() const -> const std::vector<AbstractExpressionRef> & { return group_bys_; }

  // 返回指定索引处的聚合表达式
  auto GetAggregateAt(uint32_t idx) const -> const AbstractExpressionRef & { return aggregates_[idx]; }

  // 返回聚合表达式
  auto GetAggregates() const -> const std::vector<AbstractExpressionRef> & { return aggregates_; }

  // 返回聚合类型
  auto GetAggregateTypes() const -> const std::vector<AggregationType> & { return agg_types_; }

  // 通过 group by 表达式和聚合函数推断输出模式的方法
  static auto InferAggSchema(const std::vector<AbstractExpressionRef> &group_bys,
                             const std::vector<AbstractExpressionRef> &aggregates,
                             const std::vector<AggregationType> &agg_types) -> Schema;

  // 重写抽象计划节点的克隆方法
  BUSTUB_PLAN_NODE_CLONE_WITH_CHILDREN(AggregationPlanNode);

  // 聚合的 group by 表达式
  std::vector<AbstractExpressionRef> group_bys_;
  // 聚合的表达式
  std::vector<AbstractExpressionRef> aggregates_;
  // 聚合的类型
  std::vector<AggregationType> agg_types_;

 protected:
  // 将计划节点转换为字符串表示形式
  auto PlanNodeToString() const -> std::string override;
};

// 聚合键结构体，表示聚合操作中的键
struct AggregateKey {
  // group by 的值
  std::vector<Value> group_bys_;

  // 比较两个聚合键是否相等的方法
  auto operator==(const AggregateKey &other) const -> bool {
    for (uint32_t i = 0; i < other.group_bys_.size(); i++) {
      if (group_bys_[i].CompareEquals(other.group_bys_[i]) != CmpBool::CmpTrue) {
        return false;
      }
    }
    return true;
  }
};

// 聚合值结构体，表示每个正在进行的聚合的值
struct AggregateValue {
  // 聚合值
  std::vector<Value> aggregates_;
};

}  // namespace bustub

namespace std {

// 对 AggregateKey 实现哈希方法
template <>
struct hash<bustub::AggregateKey> {
  auto operator()(const bustub::AggregateKey &agg_key) const -> std::size_t {
    size_t curr_hash = 0;
    for (const auto &key : agg_key.group_bys_) {
      if (!key.IsNull()) {
        curr_hash = bustub::HashUtil::CombineHashes(curr_hash, bustub::HashUtil::HashValue(&key));
      }
    }
    return curr_hash;
  }
};

}  // namespace std

template <>
struct fmt::formatter<bustub::AggregationType> : formatter<std::string> {
  template <typename FormatContext>
  auto format(bustub::AggregationType c, FormatContext &ctx) const {
    using bustub::AggregationType;
    std::string name = "unknown";
    switch (c) {
      case AggregationType::CountStarAggregate:
        name = "count_star";
        break;
      case AggregationType::CountAggregate:
        name = "count";
        break;
      case AggregationType::SumAggregate:
        name = "sum";
        break;
      case AggregationType::MinAggregate:
        name = "min";
        break;
      case AggregationType::MaxAggregate:
        name = "max";
        break;
    }
    return formatter<std::string>::format(name, ctx);
  }
};
