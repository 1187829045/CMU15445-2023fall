//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// extendible_htable_header_page.h
//
// Identification: src/include/storage/page/extendible_htable_header_page.h
//
// Copyright (c) 2015-2023, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

/**
 * Header page format:
 *  ---------------------------------------------------
 * | DirectoryPageIds(2048) | MaxDepth (4) | Free(2044)
 *  ---------------------------------------------------
 */

#pragma once

#include <cstdlib>
#include "common/config.h"
#include "common/macros.h"

namespace bustub {

static constexpr uint64_t HTABLE_HEADER_PAGE_METADATA_SIZE = sizeof(uint32_t);//表示哈希表头部页面的元数据大小
static constexpr uint64_t HTABLE_HEADER_MAX_DEPTH = 9;//表示哈希表的最大深度或目录页ID的最大长度
static constexpr uint64_t HTABLE_HEADER_ARRAY_SIZE = 1 << HTABLE_HEADER_MAX_DEPTH;//directory_page_ids_最大长度

class ExtendibleHTableHeaderPage {
 public:
  // Delete all constructor / destructor to ensure memory safety
  ExtendibleHTableHeaderPage() = delete;//删除所有构造函数和析构函数确保内存安全
  DISALLOW_COPY_AND_MOVE(ExtendibleHTableHeaderPage);//不允许COPY AND MOVE

  /**
   * After creating a new header page from buffer pool, must call initialize
   * method to set default values
   * @param max_depth Max depth in the header page
   */
  void Init(uint32_t max_depth = HTABLE_HEADER_MAX_DEPTH);//冲缓冲池中创建一个头页必须调用该函数去设置最大深度，若没有参数就是默认值

  /**
   * Get the directory index that the key is hashed to
   *
   * @param hash the hash of the key
   * @return directory index the key is hashed to
   */
  auto HashToDirectoryIndex(uint32_t hash) const -> uint32_t;//函数接受键的哈希值作为输入，并返回表示该键映射到的目录下标。

  /**
   * Get the directory page id at an index
   *
   * @param directory_idx index in the directory page id array
   * @return directory page_id at index
   */
  auto GetDirectoryPageId(uint32_t directory_idx) const -> uint32_t;//directory_idx 目录页ID数组的下标 得到目录页ID

  /**
   * @brief Set the directory page id at an index
   *
   * @param directory_idx index in the directory page id array
   * @param directory_page_id page id of the directory
   */
  void SetDirectoryPageId(uint32_t directory_idx, page_id_t directory_page_id);//设置directory_page_id在 directory_idx下标下

  /**
   * @brief Get the maximum number of directory page ids the header page could handle
   */
  auto MaxSize() const -> uint32_t;//获取 header page 可以处理的目录页 id 的最大数量

  /**
   * Prints the header's occupancy information
   */
  void PrintHeader() const;//打印header's occupancy information

 private:
  page_id_t directory_page_ids_[HTABLE_HEADER_ARRAY_SIZE];//存储页ID
  uint32_t max_depth_;//最大能到达的深度
};

static_assert(sizeof(page_id_t) == 4);//这个断言检查类型 page_id_t 的大小是否等于 4 个字节

static_assert(sizeof(ExtendibleHTableHeaderPage) ==
              sizeof(page_id_t) * HTABLE_HEADER_ARRAY_SIZE + HTABLE_HEADER_PAGE_METADATA_SIZE);
//这个断言检查类型 ExtendibleHTableHeaderPage 的大小是否符合预期
static_assert(sizeof(ExtendibleHTableHeaderPage) <= BUSTUB_PAGE_SIZE);
//这个断言检查类型 ExtendibleHTableHeaderPage 的大小是否小于等于预定义的 
}  // namespace bustub
