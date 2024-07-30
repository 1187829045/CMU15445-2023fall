//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// index.h
//
// Identification: src/include/storage/index/index.h
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
//这段代码定义了一个索引类 (Index) 和与之关联的元数据类 (IndexMetadata)。主要作用如下：
#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "catalog/schema.h"
#include "storage/table/tuple.h"
#include "type/value.h"

namespace bustub {

class Transaction;

/**
 * class IndexMetadata - Index 元数据的类，用于维护索引对象的元数据信息。
 *
 * 元数据对象维护索引的元组模式和键属性，因为外部调用方不知道索引键的实际结构，
 * 所以索引的责任是维护这样的映射关系，并在元组键和索引键之间进行转换。
 */
class IndexMetadata {
 public:
  IndexMetadata() = delete;

  /**
   * 构造一个新的 IndexMetadata 实例。
   * @param index_name 索引的名称
   * @param table_name 创建索引的表的名称
   * @param tuple_schema 索引键的模式
   * @param key_attrs 从索引列到基表列的映射
   */
  /*
  index_name 是索引的唯一标识符，用于在数据库中区分不同的索引。
  每个索引都应该有一个独一无二的名称，以便于识别和管理。
  table_name 则是创建索引的表的名称。它指明了在哪个表上创建了这个索引。
  通常，一个表可以拥有多个索引，每个索引都与一个特定的表关联。
  */
  IndexMetadata(std::string index_name, std::string table_name, const Schema *tuple_schema,
                std::vector<uint32_t> key_attrs, bool is_primary_key)
      : name_(std::move(index_name)),
        table_name_(std::move(table_name)),
        key_attrs_(std::move(key_attrs)),
        is_primary_key_(is_primary_key) {
    key_schema_ = std::make_shared<Schema>(Schema::CopySchema(tuple_schema, key_attrs_));
  }

  ~IndexMetadata() = default;

  /** @return 索引的名称 */
  inline auto GetName() const -> const std::string & { return name_; }

  /** @return 创建索引的表的名称 */
  inline auto GetTableName() -> const std::string & { return table_name_; }

  /** @return 表示索引键的模式的 schema 对象指针 */
  inline auto GetKeySchema() const -> Schema * { return key_schema_.get(); }

  /**
   * @return 索引键中的列数（不在元组键中）
   *
   * 注意：此方法必须在 cpp 源文件中定义，因为它使用 catalog::Schema 的成员，这里无法知晓。
   */
  auto GetIndexColumnCount() const -> std::uint32_t { return static_cast<uint32_t>(key_attrs_.size()); }

  /** @return 索引列与基表列之间的映射关系 */
  inline auto GetKeyAttrs() const -> const std::vector<uint32_t> & { return key_attrs_; }

  /** @return 是否为主键 */
  inline auto IsPrimaryKey() const -> bool { return is_primary_key_; }

  /** @return 用于调试的字符串表示 */
  auto ToString() const -> std::string {
    std::stringstream os;

    os << "IndexMetadata["
       << "Name = " << name_ << ", "
       << "Type = B+Tree, "
       << "Table name = " << table_name_ << "] :: ";
    os << key_schema_->ToString();

    return os.str();
  }

 private:
  /** 索引的名称 */
  std::string name_;
  /** 创建索引的表的名称 */
  std::string table_name_;
  /** 索引键模式与元组模式之间的映射关系 */
  const std::vector<uint32_t> key_attrs_;
  /** 索引键的模式 */
  std::shared_ptr<Schema> key_schema_;
  /** 是否为主键 */
  bool is_primary_key_;
};

/////////////////////////////////////////////////////////////////////
// Index 类定义
/////////////////////////////////////////////////////////////////////

/**
 * class Index - 派生索引的基类
 *
 * 索引结构主要维护了基础表的模式信息和索引键与元组键之间的映射关系，
 * 并提供了一个抽象的方式供外部世界与底层索引实现交互，而不暴露实际实现的接口。
 *
 * 索引对象还处理谓词扫描，除了简单的插入、删除、谓词插入、点查询和全索引扫描外。
 * 谓词扫描仅支持连接，并且可能会根据谓词中的表达式类型进行优化或不优化。
 */
class Index {
 public:
  /**
   * 构造一个新的 Index 实例。
   * @param metadata 持有索引元数据的独占指针
   */
  explicit Index(std::unique_ptr<IndexMetadata> &&metadata) : metadata_{std::move(metadata)} {}

  virtual ~Index() = default;

  /** @return 与索引关联的元数据对象的非持有指针 */
  auto GetMetadata() const -> IndexMetadata * { return metadata_.get(); }

  /** @return 索引的列数 */
  auto GetIndexColumnCount() const -> std::uint32_t { return metadata_->GetIndexColumnCount(); }

  /** @return 索引名称 */
  auto GetName() const -> const std::string & { return metadata_->GetName(); }

  /** @return 索引键模式 */
  auto GetKeySchema() const -> Schema * { return metadata_->GetKeySchema(); }

  /** @return 索引键属性 */
  auto GetKeyAttrs() const -> const std::vector<uint32_t> & { return metadata_->GetKeyAttrs(); }

  /** @return 用于调试的字符串表示 */
  auto ToString() const -> std::string {
    std::stringstream os;
    os << "INDEX: (" << GetName() << ")";
    os << metadata_->ToString();
    return os.str();
  }

  ///////////////////////////////////////////////////////////////////
  // 点修改
  ///////////////////////////////////////////////////////////////////

  /**
   * 插入一个条目到索引中。
   * @param key 索引键
   * @param rid 与键相关联的 RID
   * @param transaction 事务上下文
   * @returns 插入是否成功
   */
  virtual auto InsertEntry(const Tuple &key, RID rid, Transaction *transaction) -> bool = 0;

  /**
   * 根据键删除索引条目。
   * @param key 索引键
   * @param rid 与键相关联的 RID（未使用）
   * @param transaction 事务上下文
   */
  virtual void DeleteEntry(const Tuple &key, RID rid, Transaction *transaction) = 0;

  /**
   * 搜索所提供的键的索引。
   * @param key 索引键
   * @param result 用搜索结果填充的 RID 集合
   * @param transaction 事务上下文
   */
  virtual void ScanKey(const Tuple &key, std::vector<RID> *result, Transaction *transaction) = 0;

 private:
  /** 索引结构拥有其元数据 */
  std::unique_ptr<IndexMetadata> metadata_;
};

}  // namespace bustub
