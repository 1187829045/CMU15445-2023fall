
#include "buffer/lru_k_replacer.h"
#include <algorithm>
#include "common/exception.h"

namespace bustub {

LRUKReplacer::LRUKReplacer(size_t num_frames, size_t k) : replacer_size_(num_frames), k_(k) {
  node_store_.resize(replacer_size_);
  evictable_.resize(replacer_size_, false);
}

auto LRUKReplacer::Evict(frame_id_t *frame_id) -> bool {
  std::lock_guard<std::mutex> lock(latch_);
  current_timestamp_++;
  if (node_store_.empty()) {
    return false;
  }
  *frame_id = -1;
  std::shared_ptr<LRUKNode> node;
  size_t max_k_distance = 0;
  std::vector<std::shared_ptr<LRUKNode>> inf_k_distance_nodes;
  for (size_t fid = 0; fid < replacer_size_; fid++) {
    node = node_store_[fid];
    if (!evictable_[fid]) {
      continue;
    }
    size_t k_distance = current_timestamp_ - node->EarliestTimestamp();
    if (node->Size() < k_) {
      k_distance = LRUKReplacer::MAX_K_DISTANCE;
    }

    if (k_distance > max_k_distance) {
      *frame_id = fid;
      max_k_distance = k_distance;
    }

    if (k_distance == LRUKReplacer::MAX_K_DISTANCE) {
      inf_k_distance_nodes.emplace_back(node);
    }
  }

  if (!inf_k_distance_nodes.empty()) {
    *frame_id =
        std::min_element(inf_k_distance_nodes.begin(), inf_k_distance_nodes.end(),
                         [](const auto &a, const auto &b) { return a->EarliestTimestamp() < b->EarliestTimestamp(); })
            ->get()
            ->FrameId();
  }

  if (*frame_id == -1) {
    return false;
  }

  node_store_[*frame_id] = nullptr;
  evictable_[*frame_id] = false;
  current_size_--;
  return true;
}

void LRUKReplacer::RecordAccess(frame_id_t frame_id, [[maybe_unused]] AccessType access_type) {
  std::lock_guard<std::mutex> lock(latch_);
  if (frame_id >= static_cast<int>(replacer_size_)) {
    throw std::invalid_argument{"invalid frame id"};
  }

  if (!node_store_[frame_id]) {
    auto lru_node = std::make_shared<LRUKNode>();
    lru_node->SetFid(frame_id);
    lru_node->SetK(k_);
    node_store_[frame_id] = lru_node;
  }

  node_store_[frame_id]->RecordAccess(current_timestamp_++);
}

void LRUKReplacer::SetEvictable(frame_id_t frame_id, bool set_evictable) {  //设置帧frame_id为set_evictable
  std::lock_guard<std::mutex> lock(latch_);
  current_timestamp_++;

  if (set_evictable && !node_store_[frame_id]) {
    return;
  }

  if (set_evictable) {
    if (!evictable_[frame_id]) {
      current_size_++;
    }
    evictable_[frame_id] = true;
  } else {
    if (evictable_[frame_id]) {
      current_size_--;
    }
    evictable_[frame_id] = false;
  }
}

void LRUKReplacer::Remove(frame_id_t frame_id) {
  std::lock_guard<std::mutex> lock(latch_);
  current_timestamp_++;

  if (!node_store_[frame_id]) {
    return;
  }

  if (!evictable_[frame_id]) {
    throw std::invalid_argument{"invalid frame id"};
  }

  node_store_[frame_id] = nullptr;
  evictable_[frame_id] = false;
  current_size_--;
}

auto LRUKReplacer::Size() -> size_t {
  std::lock_guard<std::mutex> lock(latch_);
  return current_size_;
}

}  // namespace bustub