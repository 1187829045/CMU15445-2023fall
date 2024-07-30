//这个类是一个扩展哈希表索引的实现，用于数据库系统中的索引管理。它提供了对键值对的快速查找、插入和删除操作
#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "container/disk/hash/disk_extendible_hash_table.h"
#include "container/hash/hash_function.h"
#include "storage/index/index.h"

namespace bustub {

#define HASH_TABLE_INDEX_TYPE ExtendibleHashTableIndex<KeyType, ValueType, KeyComparator>

template <typename KeyType, typename ValueType, typename KeyComparator>
class ExtendibleHashTableIndex : public Index {
 public:
  ExtendibleHashTableIndex(std::unique_ptr<IndexMetadata> &&metadata, BufferPoolManager *buffer_pool_manager,
                           const HashFunction<KeyType> &hash_fn);

  ~ExtendibleHashTableIndex() override = default;
  // InsertEntry: 向索引中插入一个新的条目。它接受键值对和关联的 RID 作为参数，并返回插入是否成功的布尔值。
  auto InsertEntry(const Tuple &key, RID rid, Transaction *transaction) -> bool override;
  // /DeleteEntry: 从索引中删除指定的条目。它接受键值对和关联的 RID 作为参数，并将它们从索引中删除
  void DeleteEntry(const Tuple &key, RID rid, Transaction *transaction) override;
  // ScanKey: 扫描索引以查找与给定键匹配的所有条目。它接受键值对、结果容器和事务对象作为参数，并将与给定键匹配的所有 RID
  // 存储在结果容器中。
  void ScanKey(const Tuple &key, std::vector<RID> *result, Transaction *transaction) override;

 protected:
  // comparator for key
  KeyComparator comparator_;
  // container
  DiskExtendibleHashTable<KeyType, ValueType, KeyComparator> container_;
};

constexpr static const auto TWO_INTEGER_SIZE = 8;
using IntegerKeyType = GenericKey<TWO_INTEGER_SIZE>;
using IntegerValueType = RID;
using IntegerComparatorType = GenericComparator<TWO_INTEGER_SIZE>;
using HashTableIndexForTwoIntegerColumn =
    ExtendibleHashTableIndex<IntegerKeyType, IntegerValueType, IntegerComparatorType>;

}  // namespace bustub
