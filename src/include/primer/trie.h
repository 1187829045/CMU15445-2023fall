#pragma once

#include <algorithm>
#include <cstddef>
#include <future>  // NOLINT
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace bustub {

/// 一个特殊的类型，将阻止移动构造函数和移动赋值运算符。在 TrieStore 测试中使用。
class MoveBlocked {
 public:
  explicit MoveBlocked(std::future<int> wait) : wait_(std::move(wait)) {}

  MoveBlocked(const MoveBlocked &) = delete;
  MoveBlocked(MoveBlocked &&that) noexcept {
    if (!that.waited_) {
      that.wait_.get();
    }
    that.waited_ = waited_ = true;
  }

  auto operator=(const MoveBlocked &) -> MoveBlocked & = delete;
  auto operator=(MoveBlocked &&that) noexcept -> MoveBlocked & {
    if (!that.waited_) {
      that.wait_.get();
    }
    that.waited_ = waited_ = true;
    return *this;
  }

  bool waited_{false};
  std::future<int> wait_;
};

// 一个 TrieNode 是 Trie 中的一个节点。
class TrieNode {
 public:
  // 创建一个没有子节点的 TrieNode。
  TrieNode() = default;

  // 创建一个带有一些子节点的 TrieNode。
  explicit TrieNode(std::map<char, std::shared_ptr<const TrieNode>> children) : children_(std::move(children)) {}

  virtual ~TrieNode() = default;

  // Clone 返回此 TrieNode 的副本。如果 TrieNode 包含值，则该值被复制。此函数的返回类型是指向 TrieNode 的 unique_ptr。
  //
  // 你不能使用复制构造函数来克隆节点，因为它不知道 `TrieNode` 是否包含值。
  //
  // 注意：如果你想将 `unique_ptr` 转换为 `shared_ptr`，可以使用 `std::shared_ptr<T>(std::move(ptr))`。
  virtual auto Clone() const -> std::unique_ptr<TrieNode> { return std::make_unique<TrieNode>(children_); }

  // 一个子节点映射，其中键是键中的下一个字符，值是下一个 TrieNode。
  // 你必须在此结构中存储子节点信息。不允许从结构中删除 `const`。
  std::map<char, std::shared_ptr<const TrieNode>> children_;
  auto HasChild(const char &c) const -> bool { return children_.find(c) != children_.end(); }

  auto GetChildNode(const char &c) const -> std::shared_ptr<const TrieNode> {
    if (!HasChild(c)) {
      return nullptr;
    }
    return children_.find(c)->second;
  }
  // 表示节点是否为终端节点。
  bool is_value_node_{false};

  // 你可以在此处添加额外的字段和方法，除了存储子节点之外。但一般来说，你不需要添加额外的字段来完成此项目。
};

// 一个 TrieNodeWithValue 是一个 TrieNode，它还具有与之关联的类型 T 的值。
template <class T>
class TrieNodeWithValue : public TrieNode {
 public:
  // 创建一个没有子节点但有一个值的 Trie 节点。
  explicit TrieNodeWithValue(std::shared_ptr<T> value) : value_(std::move(value)) { this->is_value_node_ = true; }

  // 创建一个具有子节点和值的 Trie 节点。
  TrieNodeWithValue(std::map<char, std::shared_ptr<const TrieNode>> children, std::shared_ptr<T> value)
      : TrieNode(std::move(children)), value_(std::move(value)) {
    this->is_value_node_ = true;
  }

  // 覆盖 Clone 方法以同时克隆值。
  //
  // 注意：如果你想将 `unique_ptr` 转换为 `shared_ptr`，可以使用 `std::shared_ptr<T>(std::move(ptr))`。
  auto Clone() const -> std::unique_ptr<TrieNode> override {
    return std::make_unique<TrieNodeWithValue<T>>(children_, value_);
  }

  // 与此 Trie 节点关联的值。
  std::shared_ptr<T> value_;
};

// 一个 Trie 是将字符串映射到类型为 T 的值的数据结构。对 Trie 的所有操作都不应修改 Trie
// 本身。应尽可能地重用现有节点，并创建新节点来表示新 Trie。
//
// 你不允许在此项目中删除任何 `const`，或使用 `mutable` 来绕过 const 检查。
class Trie {
 private:
  // Trie 的根节点。
  std::shared_ptr<const TrieNode> root_{nullptr};

  // 使用给定的根节点创建一个新的 Trie。
  explicit Trie(std::shared_ptr<const TrieNode> root) : root_(std::move(root)) {}

 public:
  // 创建一个空的 Trie。
  Trie() = default;

  // 获取与给定键关联的值。
  // 1. 如果键不在 Trie 中，则返回 nullptr。
  // 2. 如果键在 Trie 中但类型不匹配，则返回 nullptr。
  // 3. 否则，返回值。
  template <class T>
  auto Get(std::string_view key) const -> const T *;

  // 将新的键值对放入 Trie 中。如果键已经存在，则覆盖值。
  // 返回新的 Trie。
  template <class T>
  auto Put(std::string_view key, T value) const -> Trie;

  // 从 Trie 中删除键。如果键不存在，则返回原始 Trie。
  // 否则，返回新的 Trie。
  auto Remove(std::string_view key) const -> Trie;

  // 获取 Trie 的根节点，仅应在测试用例中使用。
  auto GetRoot() const -> std::shared_ptr<const TrieNode> { return root_; }
};

}  // namespace bustub
