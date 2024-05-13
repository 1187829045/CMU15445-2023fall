#pragma once
#include <list>
#include <memory>
#include <mutex>  // NOLINT
#include <unordered_map>
#include "buffer/lru_k_replacer.h"
#include "common/config.h"
#include "recovery/log_manager.h"
#include "storage/disk/disk_scheduler.h"
#include "storage/page/page.h"
#include "storage/page/page_guard.h"

namespace bustub {

/**
 * BufferPoolManager 从内部缓冲池中读取和写入磁盘页。
 */
class BufferPoolManager {
 public:
  /**
   * @brief 创建一个新的 BufferPoolManager。
   * @param disk_manager 磁盘管理器
   * @param replacer_k LRU-K 替换器的 LookBack 常数 k
   * @param log_manager 日志管理器（仅供测试使用：nullptr = 禁用日志记录）。对于 P1，请忽略此项。
   */
  BufferPoolManager(size_t pool_size, DiskManager *disk_manager, size_t replacer_k = LRUK_REPLACER_K,
                    LogManager *log_manager = nullptr);

  ~BufferPoolManager();  //销毁现有的 BufferPoolManager。

  auto GetPoolSize() -> size_t { return pool_size_; }  //返回缓冲池的大小（帧数）

  auto GetPages() -> Page * { return pages_; }  //返回缓冲池中所有页面的指针

  /**
   TODO（P1）：添加实现

    @brief 在缓冲池中创建一个新的页面。将 page_id 设置为新页面的
   id，如果所有帧都当前正在使用且不可驱逐（换句话说，已固定），则返回 nullptr。
    应该从空闲列表或替换器中选择替换帧（始终首先从空闲列表中找到），然后调用 AllocatePage() 方法获取新的页面
   id。如果替换帧有脏页， 则应首先将其写回磁盘。还需要重置新页面的内存和元数据。

    记得通过调用 replacer.SetEvictable(frame_id, false) “固定”帧，以便替换器在 BufferPoolManager
   “取消固定”帧之前不会驱逐帧。 此外，记得为 lru-k 算法在替换器中记录帧的访问历史。

    @param[out] page_id 创建的页面的 id
    @return 如果无法创建新页面，则返回 nullptr；否则返回指向新页面的指针
   */
  auto NewPage(page_id_t *page_id) -> Page *;  //指向一个新的页面ID并返回这个页

  /**
   * TODO（P2）：添加实现
   * @brief NewPage 的 PageGuard 包装器
   * 功能应该与 NewPage 相同，只是不是返回页面指针，而是返回一个 BasicPageGuard 结构。
   * @param[out] page_id 新页面的 id
   * @return 持有新页面的 BasicPageGuard
   */
  auto NewPageGuarded(page_id_t *page_id) -> BasicPageGuard;

  /**
   * TODO（P1）：添加实现
   *
   * @brief 从缓冲池中获取请求的页面。如果 page_id
   * 需要从磁盘读取，但所有帧当前正在使用且不可驱逐（换句话说，已固定），则返回 nullptr。
   *
   * 首先在缓冲池中搜索
   * page_id。如果未找到，则从空闲列表或替换器中选择替换帧（始终首先从空闲列表中找到），通过调度一个读取 DiskRequest 与
   * disk_scheduler_->Schedule() 方法从磁盘读取页面，并替换帧中的旧页面。与 NewPage()
   * 类似，如果旧页面是脏页，则需要将其写回磁盘并更新新页面的元数据。
   *
   * 此外，记得禁止驱逐并记录帧的访问历史，就像你为 NewPage() 所做的那样。
   *
   * @param page_id 要获取的页面的 id
   * @param access_type 页面的访问类型，仅用于排行榜测试。
   * @return 如果无法获取页面，则返回 nullptr；否则返回指向请求页面的指针
   */
  auto FetchPage(page_id_t page_id, AccessType access_type = AccessType::Unknown)
      -> Page *;  //查询page_id 对应的页的页信息,不存在虚拟页就创建然后再返回

  /**
   * TODO（P2）：添加实现
   *
   * @brief FetchPage 的 PageGuard 包装器
   *
   * 功能应该与 FetchPage 相同，只是根据调用的函数，返回一个保护器。
   * 如果调用 FetchPageRead 或 FetchPageWrite，则预期返回的页面已经保持了读或写锁定状态。
   *
   * @param page_id 要获取的页面的 id
   * @return 持有获取页面的 PageGuard
   */
  auto FetchPageBasic(page_id_t page_id) -> BasicPageGuard;
  auto FetchPageRead(page_id_t page_id) -> ReadPageGuard;
  auto FetchPageWrite(page_id_t page_id) -> WritePageGuard;

  /**
   * TODO（P1）：添加实现
   *
   * @brief 从缓冲池中取消固定目标页面。如果页面不在缓冲池中或其固定计数已经为 0，则返回 false。
   *
   * 减少页面的固定计数。如果固定计数达到 0，则替换器应该能够驱逐该帧。
   * 还应该根据需要设置页面的脏标志，以指示页面是否已修改。
   *
   * @param page_id 要取消固定的页面的 id
   * @param is_dirty 如果页面应该标记为脏页，则为 true；否则为 false
   * @param access_type 页面的访问类型，仅用于排行榜测试。
   * @return 如果页面不在页表中或其固定计数在此调用之前为 <= 0，则返回 false；否则返回 true
   */
  auto UnpinPage(page_id_t page_id, bool is_dirty, AccessType access_type = AccessType::Unknown) -> bool;
  //从缓冲池中取消固定目标页面。如果页面不在缓冲池中或其固定计数已经为 0，则返回 false。
  /**
   * TODO（P1）：添加实现
   *
   * @brief 将目标页面刷新到磁盘。
   *
   * 使用 DiskManager::WritePage() 方法刷新页面到磁盘，不管页面是否为脏页。
   * 刷新后取消页面的脏标志。
   *
   * @param page_id 要刷新的页面的 id，不能为 INVALID_PAGE_ID
   * @return 如果页面无法在页表中找到，则返回 false；否则返回 true
   */
  auto FlushPage(page_id_t page_id) -> bool;  //刷新页面到磁盘

  void FlushAllPages();  // 刷新缓冲池中的所有页面到磁盘。

  /**
   * TODO（P1）：添加实现
   * @brief 从缓冲池中删除一个页面。如果页面不在缓冲池中，将不执行任何操作并返回
   * true。如果页面已固定且无法删除，则立即返回 false。 从页表中删除页面后，停止跟踪替换器中的帧，并将帧添加回空闲列表。
   * 此外，重置页面的内存和元数据。最后，应调用 DeallocatePage() 来模拟释放磁盘上的页面。
   *
   * @param page_id 要删除的页面的 id
   * @return 如果页面存在但无法删除，则返回 false；如果页面不存在或删除成功，则返回 true
   */
  auto DeletePage(page_id_t page_id) -> bool;

 private:
  const size_t pool_size_;                                                     /** 缓冲池中的页面数量。 */
  std::atomic<page_id_t> next_page_id_ = 0;                                    /** 下一个要分配的页面 id */
  Page *pages_;                                                                // 缓冲池页面的数组
  std::unique_ptr<DiskScheduler> disk_scheduler_ __attribute__((__unused__));  // 指向磁盘调度器的指针
  /** 指向日志管理器的指针。对于 P1，请忽略此项。 */
  LogManager *log_manager_ __attribute__((__unused__));
  std::unordered_map<page_id_t, frame_id_t> page_table_;  //正在使用的页和物理页的映射关系
  std::unique_ptr<LRUKReplacer> replacer_;                //没有空闲页表帧只能替换，可替换页表帧
  std::list<frame_id_t> free_list_;                       //缓冲池中空闲页表帧
  /** 此锁保护共享数据结构。我们建议更新此注释以描述它所保护的内容。 */
  std::mutex latch_;
  /**
   * @brief 在磁盘上分配一个页面。调用者应在调用此函数之前获取锁。
   * @return 分配页面的 id
   */
  auto AllocatePage() -> page_id_t;

  /**
   * @brief 在磁盘上释放一个页面。调用者应在调用此函数之前获取锁。
   * @param page_id 要释放的页面的 id
   */
  void DeallocatePage(__attribute__((unused)) page_id_t page_id) {
    // 目前这个函数没有实现更复杂的数据结构来跟踪已释放的页面，因此什么也不做
  }
};
}  // namespace bustub
