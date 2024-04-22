//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// extendible_htable_header_page.cpp
//
// Identification: src/storage/page/extendible_htable_header_page.cpp
//
// Copyright (c) 2015-2023, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "storage/page/extendible_htable_header_page.h"

#include "common/exception.h"

namespace bustub {

void ExtendibleHTableHeaderPage::Init(uint32_t max_depth) {
    this->max_depth_=max_depth;
    uint32_t len=MaxSize();
    for(uint32_t i=0;i<len;i++){
      directory_page_ids_[i]=INVALID_PAGE_ID;
    }
}

auto ExtendibleHTableHeaderPage::HashToDirectoryIndex(uint32_t hash) const -> uint32_t { 
  uint32_t idx =hash >> (32 - max_depth_);//取最高的max_depth_位
  return idx;
 }

auto ExtendibleHTableHeaderPage::GetDirectoryPageId(uint32_t directory_idx) const -> uint32_t {
  assert(directory_idx < MaxSize());
  return directory_page_ids_[directory_idx];
 }

void ExtendibleHTableHeaderPage::SetDirectoryPageId(uint32_t directory_idx, page_id_t directory_page_id) {
    assert(directory_idx < MaxSize());
    directory_page_ids_[directory_idx]=directory_page_id;
}

auto ExtendibleHTableHeaderPage::MaxSize() const -> uint32_t { 
  return (1 << max_depth_);
 }

}  // namespace bustub
