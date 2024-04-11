
#pragma once

#include <limits>
#include <list>
#include <memory>
#include <mutex>  // NOLINT
#include <unordered_map>
#include <vector>

#include "common/config.h"
#include "common/macros.h"

namespace bustub {

enum class AccessType { Unknown = 0, Lookup, Scan, Index };

class LRUKNode {
 public:
  auto FrameId() const -> size_t { return fid_; }

  void SetFid(frame_id_t fid) { fid_ = fid; }

  void SetK(size_t k) { k_ = k; }  //设置大小

  void RecordAccess(size_t timestamp) {  //记录当前时间戳，并防止大小不超限
    history_.emplace_back(timestamp);
    if (history_.size() > k_) {
      history_.pop_front();
    }
  }

  auto EarliestTimestamp() const -> size_t { return history_.front(); }  //当前页面被访问或者写的时间的最早值

  auto Size() const -> size_t { return history_.size(); }  //被访问的次数

 private:
  frame_id_t fid_;
  std::list<size_t> history_;
  size_t k_;
};

/**
 * LRUKReplacer 实现了 LRU-k 替换策略。
 *
 * LRU-k 算法会驱逐其后向 k-距离最大的帧。
 * 后向 k-距离是当前时间戳和第 k 个先前访问的时间戳之间的差值。
 *
 * 对于历史引用少于 k 次的帧，将其后向 k-距离视为 +inf。
 * 如果有多个帧的后向 k-距离为 +inf，则基于 LRU 选择受害者。
 */
class LRUKReplacer {
 public:
  /**
   *
   * TODO（P1）：添加实现
   *
   * @brief 创建一个新的 LRUKReplacer。
   * @param num_frames LRUReplacer 将要存储的最大帧数
   */
  explicit LRUKReplacer(size_t num_frames, size_t k);

  DISALLOW_COPY_AND_MOVE(LRUKReplacer);

  /**
   * TODO（P1）：添加实现
   *
   * @brief 销毁 LRUReplacer。
   */
  ~LRUKReplacer() = default;

  /**
   * TODO（P1）：添加实现
   *
   * @brief 查找后向 k-距离最大的帧并驱逐该帧。只有被标记为“可驱逐”的帧才是驱逐候选。
   *
   * 如果一个帧的历史引用少于 k 次，则将其后向 k-距离视为 +inf。
   * 如果多个帧的后向 k-距离为 inf，则基于 LRU 选择受害者。
   *
   * 成功驱逐一个帧应该将替换器的大小减小，并删除该帧的访问历史记录。
   *
   * @param[out] frame_id 被驱逐的帧的 id。
   * @return 如果成功驱逐了一个帧，则返回 true；如果没有帧可以驱逐，则返回 false。
   */
  auto Evict(frame_id_t *frame_id) -> bool;  //找到一个可以被替换的页面帧并把页面帧Id赋值为frame_id

  /**
   * TODO（P1）：添加实现
   *
   * @brief 记录给定帧 id 在当前时间戳被访问的事件。
   * 如果帧 id 尚未见过，则创建一个新的访问历史记录。
   *
   * 如果帧 id 无效（即大于 replacer_size_），则抛出异常。
   * 您也可以使用 BUSTUB_ASSERT 如果帧 id 无效，则中止进程。
   *
   * @param frame_id 收到新访问的帧的 id。
   * @param access_type 收到的访问类型。此参数仅用于排行榜测试。
   */
  void RecordAccess(frame_id_t frame_id,
                    AccessType access_type = AccessType::Unknown);  //记录frame_id页面帧的访问时间戳

  /**
   * TODO（P1）：添加实现
   *
   * @brief 切换帧是否可驱逐或不可驱逐。此函数还控制替换器的大小。大小等于可驱逐条目的数量。
   *
   * 如果一个帧以前是可驱逐的，并且要设置为不可驱逐，则大小应该减小。如果一个帧以前是不可驱逐的，并且要设置为可驱逐，则大小应该增加。
   *
   * 如果帧 id 无效，则抛出异常或中止进程。
   *
   * 对于其他情况，此函数应在不修改任何内容的情况下终止。
   *
   * @param frame_id 将修改“可驱逐”状态的帧的 id。
   * @param set_evictable 指定帧是否可驱逐。
   */
  void SetEvictable(frame_id_t frame_id, bool set_evictable);  //将一个页面帧ID 和 是否可替换加入数组

  /**
   * TODO（P1）：添加实现
   *
   * @brief 从替换器中删除一个可驱逐帧，以及其访问历史记录。
   * 如果删除成功，此函数还应该将替换器的大小减小。
   *
   * 请注意，这与驱逐帧是不同的，驱逐帧总是删除具有最大后向 k-距离的帧。此函数删除指定的帧 id，无论其后向 k-距离如何。
   *
   * 如果在不可驱逐的帧上调用 Remove，则抛出异常或中止进程。
   *
   * 如果找不到指定的帧，则直接从此函数返回。
   *
   * @param frame_id 要删除的帧的 id。
   */
  void Remove(frame_id_t frame_id);

  /**
   * TODO（P1）：添加实现
   *
   * @brief 返回替换器的大小，该大小跟踪可驱逐帧的数量。
   *
   * @return size_t
   */
  auto Size() -> size_t;

 private:
  static constexpr size_t MAX_K_DISTANCE = std::numeric_limits<size_t>::max();  //最大值
  std::vector<std::shared_ptr<LRUKNode>> node_store_;                           //存储一个页面帧的数组
  std::vector<bool> evictable_;                                                 //判断是否是可以被替换
  size_t current_timestamp_{0};                                                 //当前时间戳
  size_t current_size_{0};                                                      //页面帧数量
  size_t replacer_size_;                                                        //替换器的大小
  size_t k_;
  std::mutex latch_;
};

}  // namespace bustub