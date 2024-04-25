#pragma once

#include <cassert>
#include <climits>
#include <cstdlib>
#include <string>

#include "common/config.h"
#include "storage/index/generic_key.h"

namespace bustub {

static constexpr uint64_t HTABLE_DIRECTORY_PAGE_METADATA_SIZE = sizeof(uint32_t) * 2;  //存储哈希表目录页的元数据大小

/**
 * HTABLE_DIRECTORY_ARRAY_SIZE is the number of page_ids that can fit in the directory page of an extendible hash index.
 * This is 512 because the directory array must grow in powers of 2, and 1024 page_ids leaves zero room for
 * storage of the other member variables.
 */
static constexpr uint64_t HTABLE_DIRECTORY_MAX_DEPTH = 9;                                 //目录页最大深度
static constexpr uint64_t HTABLE_DIRECTORY_ARRAY_SIZE = 1 << HTABLE_DIRECTORY_MAX_DEPTH;  //目录页数组数量

/**
 * Directory Page for extendible hash table.
 */
class ExtendibleHTableDirectoryPage {
 public:
  ExtendibleHTableDirectoryPage() = delete;  // Delete all constructor / destructor to ensure memory safety
  DISALLOW_COPY_AND_MOVE(ExtendibleHTableDirectoryPage);

  /**
   * After creating a new directory page from buffer pool, must call initialize
   * method to set default values
   * @param max_depth Max depth in the directory page
   */
  void Init(
      uint32_t max_depth =
          HTABLE_DIRECTORY_MAX_DEPTH);  //冲缓冲池中创建一个目录页必须调用该函数去设置最大深度，若没有参数就是默认值

  /**
   * Get the bucket index that the key is hashed to
   *
   * @param hash the hash of the key
   * @return bucket index current key is hashed to
   */
  auto HashToBucketIndex(uint32_t hash) const
      -> uint32_t;  ///函数接受键的哈希值作为输入，并返回表示该键被映射到的桶索引下标。

  /**
   * Lookup a bucket page using a directory index
   *
   * @param bucket_idx the index in the directory to lookup
   * @return bucket page_id corresponding to bucket_idx
   */
  auto GetBucketPageId(uint32_t bucket_idx) const -> page_id_t;  //得到桶下标对于的页ID

  /**
   * Updates the directory index using a bucket index and page_id
   *
   * @param bucket_idx directory index at which to insert page_id
   * @param bucket_page_id page_id to insert
   */
  void SetBucketPageId(uint32_t bucket_idx, page_id_t bucket_page_id);  //为桶下标设置对应的页ID

  /**
   * Gets the split image of an index
   *
   * @param bucket_idx the directory index for which to find the split image
   * //bucket_idx 要查找分割图像的目录索引
   * @return the directory index of the split image
   * //返回分割图像的目录索引
   **/
  auto GetSplitImageIndex(uint32_t bucket_idx) const -> uint32_t;
  //获取index的分割图像,return the directory index of the split image

  /**
   * GetGlobalDepthMask - returns a mask of global_depth 1's and the rest 0's.
   * In Extendible Hashing we map a key to a directory index
   * //在可扩展哈希map中我们map 键值 to 目录下标索引
   * using the following hash + mask function.
   * 使用 下面的hash +mask 函数
   * DirectoryIndex = Hash(key) & GLOBAL_DEPTH_MASK
   其中 GLOBAL_DEPTH_MASK 是一个掩码，从 LSBupwards 开始，GLOBAL_DEPTH 恰好为 1。
   例如，全局深度 3 对应于 32 位表示中的 0x00000007。
   * @return mask of global_depth 1's and the rest 0's (with 1's from LSB upwards)
   */
  auto GetGlobalDepthMask() const -> uint32_t;  // GetGlobalDepthMask - 返回全局深度 1 和其余 0 的掩码。

  /**
   * GetLocalDepthMask - same as global depth mask, except it
   * uses the local depth of the bucket located at bucket_idx
   *
   * @param bucket_idx the index to use for looking up local depth
   * @return mask of local 1's and the rest 0's (with 1's from LSB upwards)
   */
  auto GetLocalDepthMask(uint32_t bucket_idx) const -> uint32_t;  //与上面一样

  /**
   * Get the global depth of the hash table directory
   *
   * @return the global depth of the directory
   */
  auto GetGlobalDepth() const -> uint32_t;  //得到全局深度

  auto GetMaxDepth() const -> uint32_t;

  /**
   * Increment the global depth of the directory
   */
  void IncrGlobalDepth();  //增加目录的全局深度

  /**
   * Decrement the global depth of the directory
   */
  void DecrGlobalDepth();  //减少目录的全局深度

  auto CanShrink() -> bool;  // return true if the directory can be shrunk

  auto Size() const -> uint32_t;  // return the current directory size

  auto MaxSize() const -> uint32_t;  // return the max directory size

  /**
   * Gets the local depth of the bucket at bucket_idx
   *
   * @param bucket_idx the bucket index to lookup
   * @return the local depth of the bucket at bucket_idx
   */
  auto GetLocalDepth(uint32_t bucket_idx) const -> uint32_t;  //得到局部深度

  /**
   * Set the local depth of the bucket at bucket_idx to local_depth
   *将bucket_idx处的bucket的局部深度设置为local_depth
   * @param bucket_idx bucket index to update
   * @param local_depth new local depth
   */
  void SetLocalDepth(uint32_t bucket_idx, uint8_t local_depth);  //将bucket_idx处的bucket的局部深度设置为local_depth

  /**
   * Increment the local depth of the bucket at bucket_idx
   * @param bucket_idx bucket index to increment
   */
  void IncrLocalDepth(uint32_t bucket_idx);

  /**
   * Decrement the local depth of the bucket at bucket_idx
   * @param bucket_idx bucket index to decrement
   */
  void DecrLocalDepth(uint32_t bucket_idx);

  /**
   * VerifyIntegrity
   *
   * Verify the following invariants:
   * (1) All LD <= GD.
   * (2) Each bucket has precisely 2^(GD - LD) pointers pointing to it.
   * (3) The LD is the same at each index with the same bucket_page_id
   */
  void VerifyIntegrity() const;

  /**
   * Prints the current directory
   */
  void PrintDirectory() const;

 private:
  uint32_t max_depth_;
  uint32_t global_depth_;
  uint8_t local_depths_[HTABLE_DIRECTORY_ARRAY_SIZE];       //目录页的桐数组的局部深度
  page_id_t bucket_page_ids_[HTABLE_DIRECTORY_ARRAY_SIZE];  //指向每个桶的页数组
};
static_assert(sizeof(page_id_t) == 4);
static_assert(sizeof(ExtendibleHTableDirectoryPage) == HTABLE_DIRECTORY_PAGE_METADATA_SIZE +
                                                           HTABLE_DIRECTORY_ARRAY_SIZE +
                                                           sizeof(page_id_t) * HTABLE_DIRECTORY_ARRAY_SIZE);
static_assert(sizeof(ExtendibleHTableDirectoryPage) <= BUSTUB_PAGE_SIZE);

}  // namespace bustub
