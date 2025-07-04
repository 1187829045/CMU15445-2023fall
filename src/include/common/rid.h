//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// rid.h
//
// Identification: src/include/common/rid.h
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>
#include <sstream>
#include <string>

#include "common/config.h"

namespace bustub {

class RID {
 public:
  /** 默认构造函数创建一个无效的 RID！ */
  RID() = default;

  /**
   * 创建给定页标识符和槽编号的新记录标识符。
   * @param page_id 页标识符
   * @param slot_num 槽编号
   */
  RID(page_id_t page_id, uint32_t slot_num) : page_id_(page_id), slot_num_(slot_num) {}

  explicit RID(int64_t rid) : page_id_(static_cast<page_id_t>(rid >> 32)), slot_num_(static_cast<uint32_t>(rid)) {}

  inline auto Get() const -> int64_t { return (static_cast<int64_t>(page_id_)) << 32 | slot_num_; }

  inline auto GetPageId() const -> page_id_t { return page_id_; }

  inline auto GetSlotNum() const -> uint32_t { return slot_num_; }

  inline void Set(page_id_t page_id, uint32_t slot_num) {
    page_id_ = page_id;
    slot_num_ = slot_num;
  }

  inline auto ToString() const -> std::string {
    std::stringstream os;
    os << "页标识符: " << page_id_;
    os << " 槽编号: " << slot_num_ << "\n";

    return os.str();
  }

  friend auto operator<<(std::ostream &os, const RID &rid) -> std::ostream & {
    os << rid.ToString();
    return os;
  }

  auto operator==(const RID &other) const -> bool { return page_id_ == other.page_id_ && slot_num_ == other.slot_num_; }

 private:
  page_id_t page_id_{INVALID_PAGE_ID};
  uint32_t slot_num_{0};  // 逻辑偏移量从0开始，依次递增...
};

}  // namespace bustub

namespace std {
template <>
struct hash<bustub::RID> {
  auto operator()(const bustub::RID &obj) const -> size_t { return hash<int64_t>()(obj.Get()); }
};
}  // namespace std
