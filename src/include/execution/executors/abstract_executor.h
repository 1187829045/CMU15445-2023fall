//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// abstract_executor.h
//
// Identification: src/include/execution/executors/abstract_executor.h
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include "execution/executor_context.h"
#include "storage/table/tuple.h"

namespace bustub {
class ExecutorContext;
/**
 * AbstractExecutor 实现了 Volcano 一次元组迭代器模型。
 * 这是BustTub中所有执行器执行的基类
 * 引擎继承，并定义了所有执行器支持的最小接口。
 */
class AbstractExecutor {
 public:
  /**
   * 构造一个新的 AbstractExecutor 实例。
   * @param exec_ctx 执行器运行时使用的执行器上下文
   */
  explicit AbstractExecutor(ExecutorContext *exec_ctx) : exec_ctx_{exec_ctx} {}

  /** Virtual destructor. */
  virtual ~AbstractExecutor() = default;

  /**
   * 初始化执行器。
   * @warning 该函数必须在调用 Next() 之前调用！
   */
  virtual void Init() = 0;

  /**
   * 从此执行器产生下一个元组。
   * @param[out] tuple 此执行器生成的下一个元组
   * @param[out] rid 该执行器生成的下一个元组 RID
   * 如果生成了元组则返回`true`，如果没有更多的元组则返回`false`
   */
  virtual auto Next(Tuple *tuple, RID *rid) -> bool = 0;

  /** @return该执行器生成的元组的模式 */
  virtual auto GetOutputSchema() const -> const Schema & = 0;

  /** @return该执行器运行的执行器上下文 */
  auto GetExecutorContext() -> ExecutorContext * { return exec_ctx_; }

 protected:
  /** 执行器运行的执行器上下文*/
  ExecutorContext *exec_ctx_;
};
}  // namespace bustub
