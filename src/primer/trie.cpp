#include "primer/trie.h"
#include <stack>
#include <string_view>
#include "common/exception.h"

namespace bustub {

template <class T>
auto Trie::Get(std::string_view key) const -> const T * {
  // 你应该遍历 Trie 来找到与 key 对应的节点。如果节点不存在，则返回 nullptr。找到节点后，你应该使用 `dynamic_cast`
  // 将其转换为 `const TrieNodeWithValue<T> *`。如果 dynamic_cast 返回 `nullptr`，则表示值的类型不匹配，应该返回
  // nullptr。否则，返回值。

  if (root_ == nullptr) {
    return nullptr;
  }

  auto cur_node = root_;
  for (auto &c : key) {
    if (!(cur_node->HasChild(c))) {
      return nullptr;
    }

    cur_node = cur_node->GetChildNode(c);
    if (cur_node == nullptr) {
      return nullptr;
    }
  }
  if (key.empty()) {
    cur_node = cur_node->GetChildNode(*key.begin());
  }

  if (!cur_node->is_value_node_) {
    return nullptr;
  }

  auto cur_node_with_value = std::dynamic_pointer_cast<const TrieNodeWithValue<T>>(cur_node);
  if (cur_node_with_value == nullptr) {
    return nullptr;
  }

  return cur_node_with_value->value_.get();
}

template <class T>
auto Trie::Put(std::string_view key, T value) const -> Trie {
  // 注意，`T` 可能是不可复制的类型。在创建该值的 `shared_ptr` 时始终使用 `std::move`。

  // 你应该遍历 Trie，并在必要时创建新节点。如果与 key 对应的节点已存在，则应创建新的 `TrieNodeWithValue`。

  std::shared_ptr<TrieNode> new_root;
  if (root_ != nullptr) {
    new_root = root_->Clone();
  } else {
    new_root = std::make_shared<TrieNode>();
  }

  auto parent_node = new_root;
  std::shared_ptr<TrieNode> new_node;
  auto it = key.begin();
  for (; it != key.end() && std::next(it) != key.end(); ++it) {
    if (!(parent_node->HasChild(*it))) {
      new_node = std::make_shared<TrieNode>();
    } else {
      new_node = parent_node->GetChildNode(*it)->Clone();
    }
    parent_node->children_[*it] = new_node;
    parent_node = new_node;
  }

  auto value_ptr = std::make_shared<T>(std::move(value));

  if (key.empty() || !parent_node->HasChild(*it)) {
    parent_node->children_[*it] = std::make_shared<TrieNodeWithValue<T>>(value_ptr);
  } else {
    auto child_node = parent_node->GetChildNode(*it);
    parent_node->children_[*it] = std::make_shared<TrieNodeWithValue<T>>(child_node->children_, value_ptr);
  }

  auto new_trie = std::make_shared<Trie>();
  new_trie->root_ = new_root;
  return *new_trie;
}

auto Trie::Remove(std::string_view key) const -> Trie {
  // 你应该遍历 Trie，并在必要时移除节点。如果节点不再包含值，则应将其转换为
  // `TrieNode`。如果节点不再有子节点，则应将其移除。

  if (root_ == nullptr) {
    return {};
  }

  std::stack<std::shared_ptr<const TrieNode>> node_stack;

  auto cur_node = root_;
  for (auto &c : key) {
    node_stack.push(cur_node);

    if (!(cur_node->HasChild(c))) {
      return Trie(root_);
    }

    cur_node = cur_node->GetChildNode(c);
    if (cur_node == nullptr) {
      return Trie(root_);
    }
  }
  if (key.empty()) {
    cur_node = cur_node->GetChildNode(*key.begin());
  }

  if (!cur_node->is_value_node_) {
    return Trie(root_);
  }

  auto new_node = std::make_shared<const TrieNode>(cur_node->children_);
  for (auto rit = key.rbegin(); rit != key.rend(); ++rit) {
    auto new_parent = node_stack.top()->Clone();
    if (new_node->children_.empty() && !new_node->is_value_node_) {
      new_parent->children_.erase(*rit);
    } else {
      new_parent->children_[*rit] = new_node;
    }
    new_node = std::move(new_parent);
    node_stack.pop();
  }

  if (new_node->children_.empty()) {
    return {};
  }
  return Trie(new_node);
}

// 以下是模板函数的显式实例化。
//
// 通常，人们会将模板类和函数的实现写在头文件中。然而，我们将实现分离到一个 .cpp 文件中，以便更清晰。为了使编译器知道
// 模板函数的实现，我们需要在此处显式实例化它们，以便它们可以被链接器捕获。

template auto Trie::Put(std::string_view key, uint32_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint32_t *;

template auto Trie::Put(std::string_view key, uint64_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint64_t *;

template auto Trie::Put(std::string_view key, std::string value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const std::string *;

// 如果你的解决方案无法为非复制测试编译，可以删除下面的行以获得部分分数。

using Integer = std::unique_ptr<uint32_t>;

template auto Trie::Put(std::string_view key, Integer value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const Integer *;

template auto Trie::Put(std::string_view key, MoveBlocked value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const MoveBlocked *;

}  // namespace bustub
