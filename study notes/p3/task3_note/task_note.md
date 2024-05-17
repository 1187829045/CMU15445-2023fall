哈希连接算法为 HashJoinExecutor 实现内连接和左连接。此运算符的输出模式是左表中的所有列，后跟右表中的所有列。与聚合一样，你可以假设连接使用的哈希表完全适合内存。应正确处理多个元组发生哈希碰撞的情况（无论是连接的左侧还是右侧）。需要使用 HashJoinPlanNode 中的 GetLeftJoinKey() 和 GetRightJoinKey() 函数来构造连接的左侧和右侧的连接键。
提示：你需要一种方法来对具有多个属性的元组进行哈希，以便构造一个唯一的键。作为起点，可以查看 AggregationExecutor 中的 SimpleAggregationHashTable 是如何实现这个功能的。
提示：与聚合一样，哈希连接的构建侧是管道断点。你应该再次考虑哈希连接的构建阶段应该在 HashJoinExecutor::Init() 还是 HashJoinExecutor::Next() 中执行。

思路，首先要求是hash join 那么通过左键找到相匹配的右键。左键可以通过自定义函数  GetLeftJoinKey获得，然后创建一个数组存储所有的右键，右键通过自定义函数GetRightJoinKey获得。既然要通过左键找到相等的右键所以是需要进行重载==符号的。Next函数，先判断有没有匹配的右键且迭代器没有到数组末尾，就进行链接。若没有匹配的右值且为左连接就直接加入NULL。如果是已经遍历到末尾则得到下一个left_tuple,然后又得到左键，又通过左键得到匹配的右键，并存放到数组，然后又重复上述步骤。
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
第二个小任务是将 NestedLoopJoin优化为 HashJoin哈希连接通常比嵌套循环连接（Nested Loop Join）具有更好的性能。当连接谓词是由两列之间的若干个等值条件组成的合取（conjunction）时，可以使用哈希连接算法。在这个项目中，需要能够处理由 AND 连接的多个等值条件。
提示：
确保检查每个等值条件中列所属的表。可能存在外部表的列在等值条件右侧的情况。你可能会发现 ColumnValueExpression::GetTupleIdx 方法很有帮助。
应用优化器规则的顺序很重要。例如，你需要在过滤器和 NestedLoopJoin 合并后将 NestedLoopJoin 优化为 HashJoin。
当处理多个等值条件时，尝试递归地提取键，而不是通过多层 if 语句匹配连接条件。

实现思路 
当连接条件是等值连接时，哈希连接通常比嵌套循环连接更高效。代码通过解析连接谓词并提取连接键，构建新的哈希连接计划节点来替换原来的嵌套循环连接计划节点。建立一个ParseAndExpression 函数。解析一个逻辑表达式（特别是连接谓词），并提取出左右两侧的关键表达式（连接键）。
谓词转换为逻辑表达式（如 AND、OR、NOT）。如果成功转换，则递归处理其左子表达式和右子表达式。
auto *logic_expression_ptr = dynamic_cast<LogicExpression *>(predicate.get());
if (logic_expression_ptr != nullptr) {
  ParseAndExpression(logic_expression_ptr->GetChildAt(0), left_key_expressions, right_key_expressions);
  ParseAndExpression(logic_expression_ptr->GetChildAt(1), left_key_expressions, right_key_expressions);
}
尝试将谓词转换为比较表达式（如 =）。
如果成功转换，则提取左右子表达式，并根据 GetTupleIdx 判断列是属于左表还是右表。
auto *comparison_ptr = dynamic_cast<ComparisonExpression *>(predicate.get());
if (comparison_ptr != nullptr) {
  auto column_value_1 = dynamic_cast<const ColumnValueExpression &>(*comparison_ptr->GetChildAt(0));
  if (column_value_1.GetTupleIdx() == 0) {
    left_key_expressions->emplace_back(comparison_ptr->GetChildAt(0));
    right_key_expressions->emplace_back(comparison_ptr->GetChildAt(1));
  } else {
    left_key_expressions->emplace_back(comparison_ptr->GetChildAt(1));
    right_key_expressions->emplace_back(comparison_ptr->GetChildAt(0));
  }
}
递归优化子计划节点：
std::vector<AbstractPlanNodeRef> optimized_children;
for (const auto &child : plan->GetChildren()) {
  optimized_children.emplace_back(OptimizeNLJAsHashJoin(child));
}
原因：在优化当前计划节点之前，先递归地优化其子计划节点。这种自顶向下的递归方式确保了所有子节点都已经被优化，从而在处理父节点时可以假设其子节点是最优的。

克隆计划节点：
auto optimized_plan = plan->CloneWithChildren(std::move(optimized_children));
原因：在递归优化了子节点后，克隆当前计划节点，并替换其子节点为优化后的子节点。这样可以在不修改原始计划节点的情况下，生成一个新的优化计划节点。

检查并转换嵌套循环连接：
if (optimized_plan->GetType() == PlanType::NestedLoopJoin) {
  const auto &join_plan = dynamic_cast<const NestedLoopJoinPlanNode &>(*optimized_plan);
  auto predicate = join_plan.Predicate();
  std::vector<AbstractExpressionRef> left_key_expressions;
  std::vector<AbstractExpressionRef> right_key_expressions;
  ParseAndExpression(predicate, &left_key_expressions, &right_key_expressions);
  return std::make_shared<HashJoinPlanNode>(join_plan.output_schema_, join_plan.GetLeftPlan(),
                                            join_plan.GetRightPlan(), left_key_expressions, right_key_expressions,
                                            join_plan.GetJoinType());
}
原因：如果当前计划节点是 NestedLoopJoin 类型，则尝试将其转换为 HashJoin。为此，需要解析连接谓词并提取连接键，然后构建一个新的 HashJoinPlanNode。
调用 ParseAndExpression：解析连接谓词并提取左右连接键表达式。
构建 HashJoinPlanNode：使用提取的连接键表达式和原始计划节点的信息，构建并返回新的 HashJoinPlanNode。


返回优化后的计划节点：
return optimized_plan;
原因：如果当前计划节点不是 NestedLoopJoin，则直接返回优化后的计划节点。这确保了非 NestedLoopJoin 节点不受影响，同时保留了其优化后的子节点。
