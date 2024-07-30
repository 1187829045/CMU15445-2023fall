//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// index_scan_plan.h
//
// Identification: src/include/execution/plans/index_scan_plan.h
//
// Copyright (c) 2015-19, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

//代码定义了一个名为 IndexScanPlanNode
//的类，这个类的作用是表示应该进行索引扫描的表，并且可以选择性地应用一个谓词进行扫描。
#pragma once

#include <string>
#include <utility>

#include "catalog/catalog.h"
#include "concurrency/transaction.h"
#include "execution/expressions/abstract_expression.h"
#include "execution/expressions/constant_value_expression.h"
#include "execution/plans/abstract_plan.h"
namespace bustub {
/**
 * IndexScanPlanNode 表示应该使用可选谓词扫描的表。
 */
class IndexScanPlanNode : public AbstractPlanNode {
 public:
  /**
   * 创建一个新的索引扫描计划节点，带有过滤谓词。
   * @param output 此扫描计划节点的输出格式
   * @param table_oid 要扫描的表的标识符
   * @param filter_predicate 推送到索引扫描的谓词
   * @param pred_key 用于点查找的键
   */
  IndexScanPlanNode(SchemaRef output, table_oid_t table_oid, index_oid_t index_oid,
                    AbstractExpressionRef filter_predicate = nullptr, ConstantValueExpression *pred_key = nullptr)
      : AbstractPlanNode(std::move(output), {}),
        table_oid_(table_oid),
        index_oid_(index_oid),
        filter_predicate_(std::move(filter_predicate)),
        pred_key_(pred_key) {}

  auto GetType() const -> PlanType override { return PlanType::IndexScan; }

  /** @return 应扫描的表的标识符 */
  auto GetIndexOid() const -> index_oid_t { return index_oid_; }

  BUSTUB_PLAN_NODE_CLONE_WITH_CHILDREN(IndexScanPlanNode);

  /** 创建索引的表。 */
  table_oid_t table_oid_;

  /** 应扫描其元组的索引。 */
  index_oid_t index_oid_;

  /** 用于索引扫描中的过滤谓词。
   * 对于 2023 年秋季学期，在实现了序列扫描到索引扫描的优化器规则后，
   * 我们可以使用此谓词进行索引点查找。
   */
  AbstractExpressionRef filter_predicate_;

  /**
   * 用于查找的常量值键。
   * 例如，当处理“WHERE v = 1”时，我们可以在这里存储常量值 1。
   */
  const ConstantValueExpression *pred_key_;

  // 在此处添加您想要的任何内容以进行索引查找

 protected:
  auto PlanNodeToString() const -> std::string override {
    if (filter_predicate_) {
      return fmt::format("IndexScan {{ index_oid={}, filter={} }}", index_oid_, filter_predicate_);
    }
    return fmt::format("IndexScan {{ index_oid={} }}", index_oid_);
  }
};

}  // namespace bustub
