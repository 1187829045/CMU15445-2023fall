#pragma once

#include "storage/page/page.h"

namespace bustub {

class BufferPoolManager;
class ReadPageGuard;
class WritePageGuard;

/** 基本页面保护器 */
class BasicPageGuard {
 public:
  BasicPageGuard() = default;

  BasicPageGuard(BufferPoolManager *bpm, Page *page) : bpm_(bpm), page_(page) {}

  BasicPageGuard(const BasicPageGuard &) = delete;
  auto operator=(const BasicPageGuard &) -> BasicPageGuard & = delete;

  /**
   * @brief 移动构造函数
   *
   * 当调用 BasicPageGuard(std::move(other_guard)) 时，预期新的保护器将与其他保护器行为相同。
   * 此外，旧的页面保护器不应该再可用。例如，不应该同时在两个页面保护器上调用 .Drop() 并使 pin 计数减少 2。
   */
  BasicPageGuard(BasicPageGuard &&that) noexcept;

  /**
   * @brief 丢弃页面保护器
   *
   * 丢弃页面保护器应该清除所有内容（使得页面保护器不再有用），并告诉 BPM 我们已经完成对该页面的使用，符合规范要求。
   */
  void Drop();

  /**
   * @brief 移动赋值运算符
   *
   * 类似于移动构造函数，但移动赋值运算符假设 BasicPageGuard 已经有一个被保护的页面。
   * 考虑到页面保护器的目的，当一个保护器用另一个页面替换其持有的页面时，应该发生什么。
   */
  auto operator=(BasicPageGuard &&that) noexcept -> BasicPageGuard &;

  /**
   * @brief BasicPageGuard 的析构函数
   *
   * 当页面保护器超出范围时，应该表现得像页面保护器被丢弃一样。
   */
  ~BasicPageGuard();

  /**
   * @brief 将 BasicPageGuard 升级为 ReadPageGuard
   *
   * 在升级期间，受保护的页面不会从缓冲池中驱逐，并且在调用此函数后，应使基本页面保护器无效。
   *
   * @return 升级后的 ReadPageGuard
   */
  auto UpgradeRead() -> ReadPageGuard;

  /**
   * @brief 将 BasicPageGuard 升级为 WritePageGuard
   *
   * 在升级期间，受保护的页面不会从缓冲池中驱逐，并且在调用此函数后，应使基本页面保护器无效。
   *
   * @return 升级后的 WritePageGuard
   */
  auto UpgradeWrite() -> WritePageGuard;

  auto PageId() -> page_id_t { return page_->GetPageId(); }

  auto GetData() -> const char * { return page_->GetData(); }

  template <class T>
  auto As() -> const T * {
    return reinterpret_cast<const T *>(GetData());//将保护的页面数据转换为指向类型 T 的常量指针
  }

  auto GetDataMut() -> char * {
    is_dirty_ = true;
    return page_->GetData();
  }

  template <class T>
  auto AsMut() -> T * {
    return reinterpret_cast<T *>(GetDataMut());
  }

 private:
  friend class ReadPageGuard;
  friend class WritePageGuard;

  BufferPoolManager *bpm_{nullptr};//缓冲池管理器
  Page *page_{nullptr};
  bool is_dirty_{false};
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** 读取页面保护器 */
class ReadPageGuard {
 public:
  ReadPageGuard() = default;
  ReadPageGuard(BufferPoolManager *bpm, Page *page) : guard_(bpm, page) {}
  ReadPageGuard(const ReadPageGuard &) = delete;
  auto operator=(const ReadPageGuard &) -> ReadPageGuard & = delete;

  /**
   * @brief 移动构造函数
   *
   * 与 BasicPageGuard 非常相似。您想使用另一个 ReadPageGuard 创建一个 ReadPageGuard。
   * 想一想是否有任何方法可以使这个过程对自己更容易...
   */
  ReadPageGuard(ReadPageGuard &&that) noexcept;

  /**
   * @brief 移动赋值运算符
   *
   * 与 BasicPageGuard 非常相似。给定另一个 ReadPageGuard，用该 ReadPageGuard 替换此 ReadPageGuard 的内容。
   */
  auto operator=(ReadPageGuard &&that) noexcept -> ReadPageGuard &;

  /**
   * @brief 丢弃 ReadPageGuard
   *
   * ReadPageGuard 的丢弃应与 BasicPageGuard 类似，但 ReadPageGuard 有一个额外的资源 - 闩锁！
   * 但是，您应该非常仔细地考虑您想释放这些资源的顺序。
   */
  void Drop();

  /**
   * @brief ReadPageGuard 的析构函数
   *
   * 就像 BasicPageGuard 一样，这应该表现得像你丢弃了保护器一样。
   */
  ~ReadPageGuard();

  auto PageId() -> page_id_t { return guard_.PageId(); }

  auto GetData() -> const char * { return guard_.GetData(); }

  template <class T>
  auto As() -> const T * {
    return guard_.As<T>();
  }

 private:
  // 您可以选择放弃这个并添加自己的私有变量。
  BasicPageGuard guard_;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** 写入页面保护器 */
class WritePageGuard {
 public:
  WritePageGuard() = default;
  WritePageGuard(BufferPoolManager *bpm, Page *page) : guard_(bpm, page) {}
  WritePageGuard(const WritePageGuard &) = delete;
  auto operator=(const WritePageGuard &) -> WritePageGuard & = delete;

  /**
   * @brief 移动构造函数
   *
   * 与 BasicPageGuard 非常相似。您想使用另一个 WritePageGuard 创建一个 WritePageGuard。
   * 想一想是否有任何方法可以使这个过程对自己更容易...
   */
  WritePageGuard(WritePageGuard &&that) noexcept;

  /**
   * @brief 移动赋值运算符
   *
   * 与 BasicPageGuard 非常相似。给定另一个 WritePageGuard，用该 WritePageGuard 替换此 WritePageGuard 的内容。
   */
  auto operator=(WritePageGuard &&that) noexcept -> WritePageGuard &;

  /**
   * @brief 丢弃 WritePageGuard
   *
   * WritePageGuard 的丢弃应与 BasicPageGuard 类似，但 WritePageGuard 有一个额外的资源 - 闩锁！
   * 但是，您应该非常仔细地考虑您想释放这些资源的顺序。
   */
  void Drop();

  /**
   * @brief WritePageGuard 的析构函数
   *
   * 就像 BasicPageGuard 一样，这应该表现得像你丢弃了保护器一样。
   */
  ~WritePageGuard();

  auto PageId() -> page_id_t { return guard_.PageId(); }

  auto GetData() -> const char * { return guard_.GetData(); }

  template <class T>
  auto As() -> const T * {
    return guard_.As<T>();
  }

  auto GetDataMut() -> char * { return guard_.GetDataMut(); }

  template <class T>
  auto AsMut() -> T * {
    return guard_.AsMut<T>();
  }

 private:
  // 您可以选择放弃这个并添加自己的私有变量。
  BasicPageGuard guard_;
};

}  // namespace bustub
