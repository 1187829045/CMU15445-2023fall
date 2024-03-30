#include "primer/trie_store.h"
#include "common/exception.h"

namespace bustub {

template <class T>
auto TrieStore::Get(std::string_view key) -> std::optional<ValueGuard<T>> {
  // 伪代码：
  // (1) 获取根锁，获取根节点，并释放根锁。在持有根锁时不要查找 Trie 中的值。
  // (2) 在 Trie 中查找值。
  // (3) 如果找到值，则返回一个持有对值和根的引用的 ValueGuard 对象。否则，返回 std::nullopt。
  root_lock_.lock();
  auto cur_root = root_;
  root_lock_.unlock();
  auto value = cur_root.Get<T>(key);
   if (value == nullptr) {
    return std::nullopt;
  }
  return {ValueGuard<T>(std::move(cur_root), *value)};
}

template <class T>
void TrieStore::Put(std::string_view key, T value) {
  // 您需要确保一次只有一个写入者。思考如何实现这一点。逻辑应该与 `TrieStore::Get` 类似。
  std::lock_guard<std::mutex> lk(write_lock_);
  auto new_root = root_.Put<T>(key, std::move(value));
  root_lock_.lock();
  root_ = new_root;
  root_lock_.unlock();
}

void TrieStore::Remove(std::string_view key) {
  // 您需要确保一次只有一个写入者。思考如何实现这一点。逻辑应该与 `TrieStore::Get` 类似。
  std::lock_guard<std::mutex> lk(write_lock_);
  auto new_root = root_.Remove(key);
  root_lock_.lock();
  root_ = new_root;
  root_lock_.unlock();
}

// 下面是模板函数的显式实例化。

template auto TrieStore::Get(std::string_view key) -> std::optional<ValueGuard<uint32_t>>;
template void TrieStore::Put(std::string_view key, uint32_t value);

template auto TrieStore::Get(std::string_view key) -> std::optional<ValueGuard<std::string>>;
template void TrieStore::Put(std::string_view key, std::string value);

// 如果您的解决方案不能编译通过非拷贝测试，您可以删除下面的行以获得部分分数。

using Integer = std::unique_ptr<uint32_t>;

template auto TrieStore::Get(std::string_view key) -> std::optional<ValueGuard<Integer>>;
template void TrieStore::Put(std::string_view key, Integer value);

template auto TrieStore::Get(std::string_view key) -> std::optional<ValueGuard<MoveBlocked>>;
template void TrieStore::Put(std::string_view key, MoveBlocked value);

}  // namespace bustub
