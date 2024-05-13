//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// abstract_plan.h
//
// Identification: src/include/execution/plans/abstract_plan.h
//
// Copyright (c) 2015-19, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "catalog/schema.h"
#include "fmt/format.h"

namespace bustub {

#define BUSTUB_PLAN_NODE_CLONE_WITH_CHILDREN(cname)                                                          \
  auto CloneWithChildren(std::vector<AbstractPlanNodeRef> children) const->std::unique_ptr<AbstractPlanNode> \
      override {                                                                                             \
    auto plan_node = cname(*this);                                                                           \
    plan_node.children_ = children;                                                                          \
    return std::make_unique<cname>(std::move(plan_node));                                                    \
  }

/** PlanType 代表我们系统中的计划类型。 */
enum class PlanType {
  SeqScan,
  IndexScan,
  Insert,
  Update,
  Delete,
  Aggregation,
  Limit,
  NestedLoopJoin,
  NestedIndexJoin,
  HashJoin,
  Filter,
  Values,
  Projection,
  Sort,
  TopN,
  TopNPerGroup,
  MockScan,
  InitCheck,
  Window
};

class AbstractPlanNode;
using AbstractPlanNodeRef = std::shared_ptr<const AbstractPlanNode>;

/**
 * AbstractPlanNode 表示系统中所有可能类型的计划节点。
 * 计划节点被建模为树，因此每个计划节点可以有可变数量的子节点。
 * 根据 Volcano 模型，计划节点接收其子节点的元组。
 * 子节点的顺序可能很重要。
 */
class AbstractPlanNode {
 public:
  /**
   * 使用指定的输出模式和子节点创建一个新的 AbstractPlanNode。
   * @param output_schema 这个计划节点的输出模式
   * @param children 这个计划节点的子节点
   */
  AbstractPlanNode(SchemaRef output_schema, std::vector<AbstractPlanNodeRef> children)
      : output_schema_(std::move(output_schema)), children_(std::move(children)) {}

  /** 虚拟析构函数。 */
  virtual ~AbstractPlanNode() = default;

  /** @return 这个计划节点的输出模式 */
  auto OutputSchema() const -> const Schema & { return *output_schema_; }

  /** @return 此计划节点在索引 child_idx 处的子节点 */
  auto GetChildAt(uint32_t child_idx) const -> AbstractPlanNodeRef { return children_[child_idx]; }

  /** @return 这个计划节点的子节点 */
  auto GetChildren() const -> const std::vector<AbstractPlanNodeRef> & { return children_; }

  /** @return 此计划节点的类型 */
  virtual auto GetType() const -> PlanType = 0;

  /** @return 计划节点及其子节点的字符串表示 */
  auto ToString(bool with_schema = true) const -> std::string {
    if (with_schema) {
      return fmt::format("{} | {}{}", PlanNodeToString(), output_schema_, ChildrenToString(2, with_schema));
    }
    return fmt::format("{}{}", PlanNodeToString(), ChildrenToString(2, with_schema));
  }

  /** @return 具有新子节点的克隆计划节点 */
  virtual auto CloneWithChildren(std::vector<AbstractPlanNodeRef> children) const
      -> std::unique_ptr<AbstractPlanNode> = 0;

  /**
   * 这个计划节点的输出模式。在 Volcano 模型中，每个计划节点都会产生元组，
   * 这告诉您此计划节点的元组将具有什么模式。
   */
  SchemaRef output_schema_;

  /** 这个计划节点的子节点。 */
  std::vector<AbstractPlanNodeRef> children_;

 protected:
  /** @return 计划节点本身的字符串表示 */
  virtual auto PlanNodeToString() const -> std::string { return "<unknown>"; }

  /** @return 计划节点子节点的字符串表示 */
  auto ChildrenToString(int indent, bool with_schema = true) const -> std::string;

 private:
};

}  // namespace bustub

template <typename T>
struct fmt::formatter<T, std::enable_if_t<std::is_base_of<bustub::AbstractPlanNode, T>::value, char>>
    : fmt::formatter<std::string> {
  template <typename FormatCtx>
  auto format(const T &x, FormatCtx &ctx) const {
    return fmt::formatter<std::string>::format(x.ToString(), ctx);
  }
};

template <typename T>
struct fmt::formatter<std::unique_ptr<T>, std::enable_if_t<std::is_base_of<bustub::AbstractPlanNode, T>::value, char>>
    : fmt::formatter<std::string> {
  template <typename FormatCtx>
  auto format(const std::unique_ptr<T> &x, FormatCtx &ctx) const {
    return fmt::formatter<std::string>::format(x->ToString(), ctx);
  }
};
