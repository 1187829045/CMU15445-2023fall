哈希连接算法为 HashJoinExecutor 实现内连接和左连接。此运算符的输出模式是左表中的所有列，后跟右表中的所有列。与聚合一样，你可以假设连接使用的哈希表完全适合内存。应正确处理多个元组发生哈希碰撞的情况（无论是连接的左侧还是右侧）。需要使用 HashJoinPlanNode 中的 GetLeftJoinKey() 和 GetRightJoinKey() 函数来构造连接的左侧和右侧的连接键。
提示：你需要一种方法来对具有多个属性的元组进行哈希，以便构造一个唯一的键。作为起点，可以查看 AggregationExecutor 中的 SimpleAggregationHashTable 是如何实现这个功能的。
提示：与聚合一样，哈希连接的构建侧是管道断点。你应该再次考虑哈希连接的构建阶段应该在 HashJoinExecutor::Init() 还是 HashJoinExecutor::Next() 中执行。

思路，首先要求是hash join 那么通过左键找到相匹配的右键。左键可以通过自定义函数  GetLeftJoinKey获得，然后创建一个数组存储所有的右键，右键通过自定义函数GetRightJoinKey获得。既然要通过左键找到相等的右键所以是需要进行重载==符号的。Next函数，先判断有没有匹配的右键且迭代器没有到数组末尾，就进行链接。若没有匹配的右值且为左连接就直接加入NULL。如果是已经遍历到末尾则得到下一个left_tuple,然后又得到左键，又通过左键得到匹配的右键，并存放到数组，然后又重复上述步骤。