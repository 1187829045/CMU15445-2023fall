#include <cassert>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "common/config.h"
#include "common/exception.h"
#include "common/logger.h"
#include "common/macros.h"
#include "common/rid.h"
#include "common/util/hash_util.h"
#include "container/disk/hash/disk_extendible_hash_table.h"
#include "storage/index/hash_comparator.h"
#include "storage/page/extendible_htable_bucket_page.h"
#include "storage/page/extendible_htable_directory_page.h"
#include "storage/page/extendible_htable_header_page.h"
#include "storage/page/page_guard.h"
/*
思路捋一下
首先一个header会指向多个目录，一个目录会指向多个桶
header通过key的前几位映射到目录，目录通过后几位的key映射到桶
要实现的插入和移除都应该逐步hash到目标桶
*/
namespace bustub {

template <typename K, typename V, typename KC>
DiskExtendibleHashTable<K, V, KC>::DiskExtendibleHashTable(const std::string &name, BufferPoolManager *bpm,
                                                           const KC &cmp, const HashFunction<K> &hash_fn,
                                                           uint32_t header_max_depth, uint32_t directory_max_depth,
                                                           uint32_t bucket_max_size)
    : bpm_(bpm),
      cmp_(cmp),
      hash_fn_(std::move(hash_fn)),
      header_max_depth_(header_max_depth),
      directory_max_depth_(directory_max_depth),
      bucket_max_size_(bucket_max_size) {
  // std::cout << header_max_depth << directory_max_depth << bucket_max_size << "\n";
  index_name_ = name;
  // Create a new header page
  page_id_t page_id;
  auto tmp_header_guard = bpm_->NewPageGuarded(&page_id);
  auto header_guard = tmp_header_guard.UpgradeWrite();
  auto header_page = header_guard.AsMut<ExtendibleHTableHeaderPage>();
  header_page->Init(header_max_depth);
  header_page_id_ = page_id;
}

/*****************************************************************************
 * SEARCH
 *****************************************************************************/
template <typename K, typename V, typename KC>
auto DiskExtendibleHashTable<K, V, KC>::GetValue(const K &key, std::vector<V> *result, Transaction *transaction) const
    -> bool {
  auto header_guard = bpm_->FetchPageRead(header_page_id_);
  auto header_page = header_guard.As<ExtendibleHTableHeaderPage>();
  uint32_t hash = Hash(key);
  auto directory_index = header_page->HashToDirectoryIndex(hash);
  page_id_t directory_page_id = header_page->GetDirectoryPageId(directory_index);
  if (static_cast<int>(directory_page_id) == INVALID_PAGE_ID) {
    return false;
  }
  ReadPageGuard directory_guard = bpm_->FetchPageRead(directory_page_id);
  auto directory_page = directory_guard.As<ExtendibleHTableDirectoryPage>();
  auto bucket_index = directory_page->HashToBucketIndex(hash);
  page_id_t bucket_page_id = directory_page->GetBucketPageId(bucket_index);
  if (static_cast<int>(bucket_page_id) == INVALID_PAGE_ID) {
    return false;
  }
  ReadPageGuard bucket_guard = bpm_->FetchPageRead(bucket_page_id);
  auto bucket_page = bucket_guard.As<ExtendibleHTableBucketPage<K, V, KC>>();
  V value;
  bool lookup_success = bucket_page->Lookup(key, value, cmp_);
  if (!lookup_success) {
    return false;
  }
  result->push_back(value);

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename K, typename V, typename KC>
auto DiskExtendibleHashTable<K, V, KC>::Insert(const K &key, const V &value, Transaction *transaction) -> bool {
  //要插入元素先一步一步映射到桶
  auto header_guard = bpm_->FetchPageWrite(header_page_id_);
  auto header_page = header_guard.AsMut<ExtendibleHTableHeaderPage>();
  uint32_t hash = Hash(key);
  uint32_t directory_index = header_page->HashToDirectoryIndex(hash);
  auto directory_page_id = header_page->GetDirectoryPageId(directory_index);
  if (static_cast<int>(directory_page_id) == INVALID_PAGE_ID) {
    auto insert_success = InsertToNewDirectory(header_page, directory_index, hash, key, value);
    return insert_success;
  }

  header_guard.Drop();
  WritePageGuard directory_guard = bpm_->FetchPageWrite(directory_page_id);
  auto directory_page = directory_guard.AsMut<ExtendibleHTableDirectoryPage>();
  auto bucket_index = directory_page->HashToBucketIndex(hash);
  auto bucket_page_id = directory_page->GetBucketPageId(bucket_index);

  if (static_cast<int>(bucket_page_id) == INVALID_PAGE_ID) {  //要显示转换类型，使其等号左右两边相等
    auto insert_success = InsertToNewBucket(directory_page, bucket_index, key, value);
    return insert_success;
  }

  // Check whether the bucket is full or not.
  bool insert_success = false;
  WritePageGuard bucket_guard = bpm_->FetchPageWrite(bucket_page_id);
  auto bucket_page = bucket_guard.AsMut<ExtendibleHTableBucketPage<K, V, KC>>();
  V tmp_val;
  //如果已存在该键值则不需要插入直接返回false
  if (bucket_page->Lookup(key, tmp_val, cmp_)) {
    return false;
  }

  if (!bucket_page->IsFull()) {
    insert_success = bucket_page->Insert(key, value, cmp_);
    return insert_success;
  }
  //桶满了，那么扩展那个桶的局部深度，然后分裂桶，然后将值重新hash，要不停的循环可能分裂后桶在hash后依旧会满
  while (!insert_success && bucket_page->IsFull()) {
    if (directory_page->GetGlobalDepth() == directory_page->GetLocalDepth(bucket_index)) {
      if (directory_page->GetGlobalDepth() == directory_page->GetMaxDepth()) {
        return false;
      }
      // directory growing
      directory_page->IncrGlobalDepth();
    }
    //要分裂桶需要通过缓冲区重新获得一块页
    page_id_t new_bucket_page_id;
    auto tmp_new_bucket_guard = bpm_->NewPageGuarded(&new_bucket_page_id);
    auto new_bucket_guard = tmp_new_bucket_guard.UpgradeWrite();
    auto new_bucket_page = new_bucket_guard.AsMut<ExtendibleHTableBucketPage<K, V, KC>>();
    new_bucket_page->Init(bucket_max_size_);       //初始化一个新的桶页
    directory_page->IncrLocalDepth(bucket_index);  //增加局部深度
    auto new_local_depth = directory_page->GetLocalDepth(bucket_index);
    auto local_depth_mask = directory_page->GetLocalDepthMask(bucket_index);
    //因为新建了一个桶页那么要重新进行哈希
    auto new_bucket_idx =
        UpdateDirectoryMapping(directory_page, bucket_index, new_bucket_page_id, new_local_depth, local_depth_mask);

    // Rehash the old bucket's KV pairs.
    page_id_t rehash_page_id;
    std::vector<uint32_t> remove_array;
    //将当时的满桶里面的页重新hash因为增加了局部深度和分裂了桶所以需要重新分配
    for (uint32_t i = 0; i < bucket_page->Size(); ++i) {
      auto k = bucket_page->KeyAt(i);
      auto v = bucket_page->ValueAt(i);
      uint32_t hash_k = Hash(k);
      auto rehash_idx = directory_page->HashToBucketIndex(hash_k);
      rehash_page_id = directory_page->GetBucketPageId(rehash_idx);
      if (rehash_page_id == new_bucket_page_id) {
        new_bucket_page->Insert(k, v, cmp_);
        remove_array.push_back(i);
      }
    }
    auto helper = 0;  // 考虑array移除时需要删除元素的下标变化
    for (auto &remove_id : remove_array) {
      bucket_page->RemoveAt(remove_id - helper);
      helper++;
    }

    // Insert the new KV pair.
    bucket_index = directory_page->HashToBucketIndex(hash);
    rehash_page_id = directory_page->GetBucketPageId(bucket_index);
    if (rehash_page_id == new_bucket_page_id) {
      insert_success = new_bucket_page->Insert(key, value, cmp_);
      if (!insert_success && new_bucket_page->IsFull()) {
        bucket_guard = std::move(new_bucket_guard);
        bucket_page_id = new_bucket_page_id;
        bucket_page = new_bucket_page;
        bucket_index = new_bucket_idx;
      }
    } else {
      insert_success = bucket_page->Insert(key, value, cmp_);
    }
  }

  return insert_success;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename K, typename V, typename KC>
auto DiskExtendibleHashTable<K, V, KC>::InsertToNewDirectory(ExtendibleHTableHeaderPage *header, uint32_t directory_idx,
                                                             uint32_t hash, const K &key, const V &value) -> bool {
  page_id_t dir_page_id;
  auto tmp_directory_guard = bpm_->NewPageGuarded(&dir_page_id);
  auto directory_guard = tmp_directory_guard.UpgradeWrite();
  auto directory_page = directory_guard.AsMut<ExtendibleHTableDirectoryPage>();
  directory_page->Init(directory_max_depth_);
  header->SetDirectoryPageId(directory_idx, dir_page_id);
  auto bucket_idx = directory_page->HashToBucketIndex(hash);
  bool insert_success = InsertToNewBucket(directory_page, bucket_idx, key, value);

  return insert_success;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename K, typename V, typename KC>
auto DiskExtendibleHashTable<K, V, KC>::InsertToNewBucket(ExtendibleHTableDirectoryPage *directory, uint32_t bucket_idx,
                                                          const K &key, const V &value) -> bool {
  page_id_t bucket_page_id;
  auto tmp_bucket_guard = bpm_->NewPageGuarded(&bucket_page_id);
  auto bucket_guard = tmp_bucket_guard.UpgradeWrite();
  auto bucket_page = bucket_guard.AsMut<ExtendibleHTableBucketPage<K, V, KC>>();
  bucket_page->Init(bucket_max_size_);
  directory->SetBucketPageId(bucket_idx, bucket_page_id);
  directory->SetLocalDepth(bucket_idx, 0);
  assert(directory->GetLocalDepth(bucket_idx) <= directory->GetGlobalDepth());
  auto insert_success = bucket_page->Insert(key, value, cmp_);
  return insert_success;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename K, typename V, typename KC>
auto DiskExtendibleHashTable<K, V, KC>::UpdateDirectoryMapping(ExtendibleHTableDirectoryPage *directory_page,
                                                               uint32_t old_bucket_idx, page_id_t new_bucket_page_id,
                                                               uint32_t new_local_depth, uint32_t local_depth_mask)
    -> uint32_t {
  //更新映射
  auto new_first_bucket_idx = local_depth_mask & old_bucket_idx;  // 111111111& old_bucket_idx
  auto prime_idx = new_first_bucket_idx;
  uint32_t distance =
      pow(2, new_local_depth);  //这个意思就是由局部深度得到一个周期长度比如000 001 002 那么000加上8就还是000
  //过了一半减去一半，没过一半加上一半
  //从最小的下标开始 比如111就要变成011
  new_first_bucket_idx = (new_first_bucket_idx >> (new_local_depth - 1)) == 0 ? (new_first_bucket_idx + (distance / 2))
                                                                              : (new_first_bucket_idx - (distance / 2));

  for (uint32_t i = new_first_bucket_idx; i < directory_page->Size(); i += distance) {
    directory_page->SetBucketPageId(i, new_bucket_page_id);
    directory_page->SetLocalDepth(i, new_local_depth);
    directory_page->SetLocalDepth(prime_idx, new_local_depth);
    assert(directory_page->GetLocalDepth(i) <= directory_page->GetGlobalDepth());
    assert(directory_page->GetLocalDepth(prime_idx) <= directory_page->GetGlobalDepth());
    prime_idx += distance;
  }
  return new_first_bucket_idx;
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
  一个找了很久的bug：Buckets can only be merged with their split image if their split image has the same local depth.
  merge时需要调用DeletePage()吗？
  对于LD为0的情况如何处理对我来说是个难点，感觉我不太擅长处理这种边界条件。
 */
template <typename K, typename V, typename KC>
auto DiskExtendibleHashTable<K, V, KC>::Remove(const K &key, Transaction *transaction) -> bool {
  auto header_guard = bpm_->FetchPageRead(header_page_id_);
  auto header_page = header_guard.As<ExtendibleHTableHeaderPage>();
  uint32_t hash = Hash(key);  // for test
  auto directory_index = header_page->HashToDirectoryIndex(hash);
  auto directory_page_id = header_page->GetDirectoryPageId(directory_index);
  if (static_cast<int>(directory_page_id) == INVALID_PAGE_ID) {
    return false;
  }

  WritePageGuard directory_guard = bpm_->FetchPageWrite(directory_page_id);
  auto directory_page = directory_guard.AsMut<ExtendibleHTableDirectoryPage>();
  auto bucket_index = directory_page->HashToBucketIndex(hash);
  auto bucket_page_id = directory_page->GetBucketPageId(bucket_index);
  if (static_cast<int>(bucket_page_id) == INVALID_PAGE_ID) {
    return false;
  }

  // find the target key in the third level
  WritePageGuard bucket_guard = bpm_->FetchPageWrite(bucket_page_id);
  auto bucket_page = bucket_guard.AsMut<ExtendibleHTableBucketPage<K, V, KC>>();
  bool remove_success = bucket_page->Remove(key, cmp_);
  if (!remove_success) {
    return false;
  }

  while (bucket_page->IsEmpty()) {
    bucket_guard.Drop();
    auto bucket_local_depth = directory_page->GetLocalDepth(bucket_index);
    if (bucket_local_depth == 0) {
      break;
    }
    //得到分裂的桶，准备重新合并
    auto merge_bucket_index = directory_page->GetSplitImageIndex(bucket_index);
    auto merge_bucket_local_depth = directory_page->GetLocalDepth(merge_bucket_index);
    auto merge_bucket_page_id = directory_page->GetBucketPageId(merge_bucket_index);

    if (bucket_local_depth == merge_bucket_local_depth) {  //空桶和想要合并桶深度一样
      uint32_t traverse_bucket_idx =
          std::min(bucket_index & directory_page->GetLocalDepthMask(bucket_index), merge_bucket_index);
      uint32_t distance = 1 << (bucket_local_depth -
                                1);  //减小局部深度合并在一起比如 011应该和111合并在一起所以是bucket_local_depth - 1
      uint32_t new_local_depth = bucket_local_depth - 1;
      for (uint32_t i = traverse_bucket_idx; i < directory_page->Size(); i += distance) {
        directory_page->SetBucketPageId(i, merge_bucket_page_id);
        directory_page->SetLocalDepth(i, new_local_depth);
      }

      if (new_local_depth == 0) {
        break;
      }
      //假设之前是 011满了找到了111
      auto split_image_bucket_index = directory_page->GetSplitImageIndex(merge_bucket_index);  // 11->01
      auto split_image_bucket_page_id = directory_page->GetBucketPageId(split_image_bucket_index);
      WritePageGuard split_image_bucket_guard = bpm_->FetchPageWrite(split_image_bucket_page_id);
      if (split_image_bucket_page_id == INVALID_PAGE_ID) {
        break;
      }
      auto helper = bucket_page_id;
      // 我当时是咋想的？写出下面这一行抽象代码……
      // gradescope不提供测试源码，我这种复现不了测试的菜鸡重新review了一遍代码才找出这个bug。
      // directory_page->SetBucketPageId(bucket_index, 0);
      bucket_index = split_image_bucket_index;
      bucket_page_id = split_image_bucket_page_id;
      bucket_guard = std::move(split_image_bucket_guard);
      bucket_page = bucket_guard.AsMut<ExtendibleHTableBucketPage<K, V, KC>>();
      bpm_->DeletePage(helper);
    } else {
      // Can not merge because of (LD != LD(split_image))
      break;
    }
    while (directory_page->CanShrink()) {
      directory_page->DecrGlobalDepth();
    }
  }

  while (directory_page->CanShrink()) {
    directory_page->DecrGlobalDepth();
  }

  return remove_success;
}

template class DiskExtendibleHashTable<int, int, IntComparator>;
template class DiskExtendibleHashTable<GenericKey<4>, RID, GenericComparator<4>>;
template class DiskExtendibleHashTable<GenericKey<8>, RID, GenericComparator<8>>;
template class DiskExtendibleHashTable<GenericKey<16>, RID, GenericComparator<16>>;
template class DiskExtendibleHashTable<GenericKey<32>, RID, GenericComparator<32>>;
template class DiskExtendibleHashTable<GenericKey<64>, RID, GenericComparator<64>>;
}  // namespace bustub

/* 服务器端跑test，无源码，p1还没啥感觉，p2真是被折磨到了……(记录一下比较印象深刻的bug)
  BUG1: NewPage()后的没有Update取读写锁，这个bug的原因主要是写代码时对pageguard部分理解不够深刻。
  BUG2: 对page_id的初始化，应该初始化为INVALID_PAGE_ID。其实我一开始想过初始化的问题，但想当然地认为初始化为0就行，
        这样不用耗费时间来初始化。对于大多数情况下确实没问题，但在多个table时会有逻辑漏洞，要是在实际场景中情况将会更加
        复杂。代码应该是严谨的，有时候直觉犯下的小错误会make you crazy，因此在模棱两可的地方要多加思考。
  收获: 上述的bug2应该是折磨我时间最长的一个bug了，一个“靠感觉”写下的代码将会漏洞百出，谨记——严谨，意识到有地方可能出现
        bug就去思考、完善，不要带着侥幸心理。
        assert()会make sense，它一定程度上能帮助coder更好地定位bug实际位置。
        对于抽象层的把握不足，好的抽象、函数接口能帮助程序员更好地理解代码逻辑。
 */