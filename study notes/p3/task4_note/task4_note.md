第一个 Sort
先通过next得到所有元组放入容器中，然后判断是升序还是降序，然后sort排序
第二个 LimitPlanNode 指定查询将生成的元组数量；解释 SELECT * FROM __mock_table_1 LIMIT 10;
直接遍历limit次，然后选取第一个
第三个 TOP-N优化，用堆，优化跟之前的优化是差不多的。
第四个WindowFunctionExecutor,用于对列执行窗口函数。窗口函数不同于普通的聚合函数，因为它为每个输入行输出一行，并且可以与普通选择列组合使用。WindowFunctionPlanNode 中的列包含普通选择列和用于窗口函数的占位符列。