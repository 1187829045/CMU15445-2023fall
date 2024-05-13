首先阅读源码搞清代码关系，该任务是实现一个Access Method Executors(访问方法执行器)。用于后面任务的各种操作提供底层的支持。
第一个是序列扫描SeqScan：顺序扫描的输出是每个匹配元组及其原始记录标识符（RID）的副本。首先创建一个数组存放一个表里面所有元组的RID，然后通过NEXT函数遍历数组，找到下一个未被删除的元组且值符合过滤器的一个元组，

第二个是插入执行器，insert_executor：在该任务中引入子执行器，Next函数首先找到要执行插入的表GetExecutorContext()->GetCatalog()->GetTable(plan_->GetTableOid());然后得到表堆的插入函数，进行插入。首先插入有两个方面的插入，插入元组和插入索引。然后返回Tuple(返回插入到表中的行数);对于第一次调用插入，即使没有实际插入任何数据，也认为是成功的；而对于之后的调用，如果没有实际插入任何数据，则认为这次插入操作并没有成功插入任何数据。

第三个是update_executor：依旧先获取要更新的表，然后获取要更新的索引数组。根据提示，要先删除再插入实现更新。

第四个是delete_executor：依旧是跟之前的准备步骤一样，返回Tuple(存储删除的行数),删除操作用UpdateTupleMeta将meta的is_delete设置会true即可

第五个是index_scan_executor，索引扫描:这一个有点懵逼一开始，借鉴了某位不知名大佬代码。 auto *hash_index = dynamic_cast<HashTableIndexForTwoIntegerColumn *>(index_info->index_.get());使用这行代码得到hash。然后得到索引扫描右边的值，索引扫描一般形式是WHERE student_id = 123;右边的123，并转换为常量值。dynamic_cast<ConstantValueExpression *>(plan_->filter_predicate_->children_[1].get());扫描符合右值（比如123）存储在数组A中。Next扫描A数组，找到一个未被删除的元组，返回。如果都被删除则返回false。

最后一个Optimizing SeqScan to IndexScan，直接仿照merge_filter_scan.cpp，然后书写seqscan_as_indexscan.cpp里面的函数。递归优化 
 for (const auto &child : plan->GetChildren()) {
    children.emplace_back(OptimizeSeqScanAsIndexScan(child));
}
函数首先递归地遍历计划的子节点，并对子节点进行优化。这样可以确保整个计划树都被优化，接着，函数克隆了原始计划，并用优化后的子节点替换了原始的子节点，以便于后续操作。如果当前计划的类型是序列扫描（SeqScan），则进入下一步的优化。如果序列扫描节点的过滤谓词不为空，并且是一个相等比较表达式（Equal Comparison），则进入下一步的优化。这里假设只有相等比较的情况下才能进行优化，即 WHERE v1 = 1，不支持其他情况的优化。获取表格信息和所有索引信息，准备进行索引扫描的匹配。遍历表格的所有索引，对比过滤条件的列与索引列是否匹配。如果找到匹配的索引列，则创建一个索引扫描计划节点，并返回。如果没有找到匹配的索引列，或者过滤条件不是相等比较表达式，则直接返回原始的优化后的计划。