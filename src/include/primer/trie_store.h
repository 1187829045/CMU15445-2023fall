#pragma once

#include <optional>
#include <shared_mutex>
#include <utility>

#include "primer/trie.h"

namespace bustub {

// 此类用于保护 Trie 返回的值。它持有对根节点的引用，以确保值的引用不会失效。
template <class T>
class ValueGuard {
 public:
  ValueGuard(Trie root, const T &value) : root_(std::move(root)), value_(value) {}
  auto operator*() const -> const T & { return value_; }

 private:
  Trie root_;
  const T &value_;
};

// 此类是 Trie 类的线程安全封装。它提供了一个简单的接口来访问 Trie。它应该允许并发读取以及同时进行的单个写操作。
class TrieStore {
 public:
  // 此函数返回一个 ValueGuard 对象，该对象持有对 Trie 中值的引用。如果键不存在于 Trie 中，则返回 std::nullopt。
  template <class T>
  auto Get(std::string_view key) -> std::optional<ValueGuard<T>>;

  // 此函数将键值对插入到 Trie 中。如果键已经存在于 Trie 中，则覆盖值。
  template <class T>
  void Put(std::string_view key, T value);

  // 此函数将从 Trie 中删除键值对。
  void Remove(std::string_view key);

 private:
  // 此互斥量保护根节点。每次想要访问或修改 Trie 根节点时，都需要获取此锁。
  std::mutex root_lock_;

  // 此互斥量对所有写操作进行排序，并且一次只允许一个写操作。
  std::mutex write_lock_;

  // 存储当前 Trie 的根节点。
  Trie root_;
};

}  // namespace bustub
