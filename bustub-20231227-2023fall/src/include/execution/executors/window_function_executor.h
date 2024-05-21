//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// window_function_executor.h
//
// Identification: src/include/execution/executors/window_function_executor.h
//
// Copyright (c) 2015-2022, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <vector>

#include "execution/executor_context.h"
#include "execution/executors/abstract_executor.h"
#include "execution/executors/aggregation_executor.h"
#include "execution/expressions/column_value_expression.h"
#include "execution/plans/window_plan.h"
#include "storage/table/tuple.h"

namespace bustub {

/**
 * The WindowFunctionExecutor executor executes a window function for columns using window function.
 *
 * Window function is different from normal aggregation as it outputs one row for each inputing rows,
 * and can be combined with normal selected columns. The columns in WindowFunctionPlanNode contains both
 * normal selected columns and placeholder columns for window functions.
 *
 * For example, if we have a query like:
 *    SELECT 0.1, 0.2, SUM(0.3) OVER (PARTITION BY 0.2 ORDER BY 0.3), SUM(0.4) OVER (PARTITION BY 0.1 ORDER BY 0.2,0.3)
 *      FROM table;
 *
 * The WindowFunctionPlanNode contains following structure:
 *    columns: std::vector<AbstractExpressionRef>{0.1, 0.2, 0.-1(placeholder), 0.-1(placeholder)}
 *    window_functions_: {
 *      3: {
 *        partition_by: std::vector<AbstractExpressionRef>{0.2}
 *        order_by: std::vector<AbstractExpressionRef>{0.3}
 *        functions: std::vector<AbstractExpressionRef>{0.3}
 *        window_func_type: WindowFunctionType::SumAggregate
 *      }
 *      4: {
 *        partition_by: std::vector<AbstractExpressionRef>{0.1}
 *        order_by: std::vector<AbstractExpressionRef>{0.2,0.3}
 *        functions: std::vector<AbstractExpressionRef>{0.4}
 *        window_func_type: WindowFunctionType::SumAggregate
 *      }
 *    }
 *
 * Your executor should use child executor and exprs in columns to produce selected columns except for window
 * function columns, and use window_agg_indexes, partition_bys, order_bys, functionss and window_agg_types to
 * generate window function columns results. Directly use placeholders for window function columns in columns is
 * not allowed, as it contains invalid column id.
 *
 * Your WindowFunctionExecutor does not need to support specified window frames (eg: 1 preceding and 1 following).
 * You can assume that all window frames are UNBOUNDED FOLLOWING AND CURRENT ROW when there is ORDER BY clause, and
 * UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING when there is no ORDER BY clause.
 *
 */
class WindowFunctionExecutor : public AbstractExecutor {
 public:
  /**
   * Construct a new WindowFunctionExecutor instance.
   * @param exec_ctx The executor context
   * @param plan The window aggregation plan to be executed
   */
  WindowFunctionExecutor(ExecutorContext *exec_ctx, const WindowFunctionPlanNode *plan,
                         std::unique_ptr<AbstractExecutor> &&child_executor);

  /** Initialize the window aggregation */
  void Init() override;

  /**
   * Yield the next tuple from the window aggregation.
   * @param[out] tuple The next tuple produced by the window aggregation
   * @param[out] rid The next tuple RID produced by the window aggregation
   * @return `true` if a tuple was produced, `false` if there are no more tuples
   */
  auto Next(Tuple *tuple, RID *rid) -> bool override;

  /** @return The output schema for the window aggregation plan */
  auto GetOutputSchema() const -> const Schema & override { return plan_->OutputSchema(); }

 private:
  /** The window aggregation plan node to be executed */
  const WindowFunctionPlanNode *plan_;

  /** The child executor from which tuples are obtained */
  std::unique_ptr<AbstractExecutor> child_executor_;

  /** Store all the Tuples*/
  std::vector<Tuple> tuples_;

  /** Iterator for all tuples*/
  std::vector<Tuple>::iterator iter_;

  /** class used to execute the window_function*/
  class WindowAggregate {
   public:
    using TupleIterator = std::vector<Tuple>::iterator;
    WindowAggregate(const std::vector<AbstractExpressionRef> &columns, std::vector<AbstractExpressionRef> &group_bys,
                    AbstractExpressionRef &agg_expr, uint32_t function_column_idx,
                    const std::unordered_set<uint32_t> &function_column_idx_set, WindowFunctionType win_function_type,
                    const std::vector<std::pair<OrderByType, AbstractExpressionRef>> &order_bys,
                    const Schema &child_schema, const Schema &output_schema, std::vector<Tuple> &tuples)
        : columns_(columns),
          group_bys_(group_bys),
          agg_expr_(agg_expr),
          function_column_idx_(function_column_idx),
          function_column_idx_set_(function_column_idx_set),
          window_function_type_(win_function_type),
          order_by_omitted_(order_bys.empty()),
          order_bys_(order_bys),
          child_schema_(&child_schema),
          output_schema_(&output_schema),
          tuples_(tuples) {}

    // Aggregate the partition between tuples [lower_bound_iter, upper_bound_iter);
    void Compute(TupleIterator lower_bound_iter, TupleIterator upper_bound_iter, TupleIterator begin_iter) {
      AggregationType agg_type;
      Value default_value;

      switch (window_function_type_) {
        case WindowFunctionType::CountStarAggregate:
          agg_type = AggregationType::CountStarAggregate;
          default_value = ValueFactory::GetIntegerValue(0);
          break;
        case WindowFunctionType::CountAggregate:
          agg_type = AggregationType::CountAggregate;
          default_value = ValueFactory::GetNullValueByType(TypeId::INTEGER);
          break;
        case WindowFunctionType::SumAggregate:
          agg_type = AggregationType::SumAggregate;
          default_value = ValueFactory::GetNullValueByType(TypeId::INTEGER);
          break;
        case WindowFunctionType::MinAggregate:
          agg_type = AggregationType::MinAggregate;
          default_value = ValueFactory::GetNullValueByType(TypeId::INTEGER);
          break;
        case WindowFunctionType::MaxAggregate:
          agg_type = AggregationType::MaxAggregate;
          default_value = ValueFactory::GetNullValueByType(TypeId::INTEGER);
          break;
        case WindowFunctionType::Rank:
          is_simple_aggregate_ = false;
          default_value = ValueFactory::GetNullValueByType(TypeId::INTEGER);
          break;
      }

      if (is_simple_aggregate_) {
        const auto agg_exprs = std::vector<AbstractExpressionRef>{agg_expr_};
        const auto agg_types = std::vector<AggregationType>{agg_type};
        SimpleAggregationHashTable aht{agg_exprs, agg_types};
        aht.Clear();

        AggregateKey agg_key = MakeAggregateKey(&(*lower_bound_iter));

        // if order_by is omitted, do a global aggregation
        if (order_by_omitted_) {
          for (auto iter = lower_bound_iter; iter != upper_bound_iter; ++iter) {
            AggregateValue agg_value = MakeAggregateValue(&(*iter));
            aht.InsertCombine(agg_key, agg_value);
          }
        }

        for (auto iter = lower_bound_iter; iter != upper_bound_iter; ++iter) {
          auto tuple_iter = tuples_.begin() + std::distance(begin_iter, iter);

          // if order_by is valid, do a moving aggregation
          if (!order_by_omitted_) {
            AggregateValue agg_value = MakeAggregateValue(&(*iter));
            aht.InsertCombine(agg_key, agg_value);
          }

          std::vector<Value> values{};
          for (uint32_t idx = 0; idx < output_schema_->GetColumns().size(); idx++) {
            /*
             * If it is this window function column, the value is the aggregation value
             * If it is not a window function column, the value should be from the child tuple
             * If it is another window function column, keep it as is
             * Else, the tuple is still growing, let's populate a default value
             * */
            if (idx == function_column_idx_) {
              values.push_back(aht.Begin().Val().aggregates_.begin()[0]);
            } else if (function_column_idx_set_.count(idx) == 0) {
              values.push_back(columns_[idx]->Evaluate(&(*iter), *child_schema_));
            } else if (tuple_iter < tuples_.end()) {
              values.push_back(tuple_iter->GetValue(output_schema_, idx));
            } else {
              values.push_back(default_value);
            }
          }

          if (tuple_iter < tuples_.end()) {
            *tuple_iter = Tuple{values, output_schema_};
          } else {
            tuples_.emplace_back(values, output_schema_);
          }
        }
      } else {
        int global_rank = 0;
        int local_rank = 0;
        for (auto iter = lower_bound_iter; iter != upper_bound_iter; ++iter) {
          auto tuple_iter = tuples_.begin() + std::distance(begin_iter, iter);

          std::vector<Value> values{};
          for (uint32_t idx = 0; idx < output_schema_->GetColumns().size(); idx++) {
            if (idx == function_column_idx_) {
              ++global_rank;
              if (local_rank == 0 || !Equal(*iter, *(iter - 1))) {
                local_rank = global_rank;
              }
              values.push_back(ValueFactory::GetIntegerValue(local_rank));
            } else if (function_column_idx_set_.count(idx) == 0) {
              values.push_back(columns_[idx]->Evaluate(&(*iter), *child_schema_));
            } else if (tuple_iter < tuples_.end()) {
              values.push_back(tuple_iter->GetValue(output_schema_, idx));
            } else {
              values.push_back(default_value);
            }
          }

          if (tuple_iter < tuples_.end()) {
            *tuple_iter = Tuple{values, output_schema_};
          } else {
            tuples_.emplace_back(values, output_schema_);
          }
        }
      }
    }

   private:
    /** @return The tuple as an AggregateKey */
    auto MakeAggregateKey(const Tuple *tuple) -> AggregateKey {
      std::vector<Value> keys;
      for (const auto &expr : group_bys_) {
        keys.emplace_back(expr->Evaluate(tuple, *output_schema_));
      }
      return {keys};
    }

    /** @return The tuple as an AggregateValue */
    auto MakeAggregateValue(const Tuple *tuple) -> AggregateValue {
      std::vector<Value> vals;
      vals.emplace_back(agg_expr_->Evaluate(tuple, *output_schema_));
      return {vals};
    }

    auto Equal(const Tuple &a, const Tuple &b) -> bool {
      for (const auto &order_key : order_bys_) {
        switch (order_key.first) {
          case OrderByType::INVALID:  // NOLINT
          case OrderByType::DEFAULT:  // NOLINT
          case OrderByType::ASC:
            return static_cast<bool>(order_key.second->Evaluate(&a, *output_schema_)
                                         .CompareEquals(order_key.second->Evaluate(&b, *output_schema_)));
          case OrderByType::DESC:
            return static_cast<bool>(order_key.second->Evaluate(&a, *output_schema_)
                                         .CompareEquals(order_key.second->Evaluate(&b, *output_schema_)));
        }
      }
      return false;
    }

    [[maybe_unused]] const std::vector<AbstractExpressionRef> &columns_;
    std::vector<AbstractExpressionRef> &group_bys_;
    AbstractExpressionRef &agg_expr_;
    uint32_t function_column_idx_;
    const std::unordered_set<uint32_t> &function_column_idx_set_;
    WindowFunctionType window_function_type_;
    bool order_by_omitted_;
    const std::vector<std::pair<OrderByType, AbstractExpressionRef>> &order_bys_;
    const Schema *child_schema_;
    const Schema *output_schema_;
    std::vector<Tuple> &tuples_;
    bool is_simple_aggregate_{true};
  };
};
}  // namespace bustub