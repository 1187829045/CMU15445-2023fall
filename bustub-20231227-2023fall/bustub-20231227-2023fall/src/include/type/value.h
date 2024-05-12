//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// value.h
//
// Identification: src/include/type/value.h
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <cstring>
#include <memory>
#include <string>
#include <utility>

#include "fmt/format.h"

#include "type/limits.h"
#include "type/type.h"

namespace bustub {

inline auto GetCmpBool(bool boolean) -> CmpBool { return boolean ? CmpBool::CmpTrue : CmpBool::CmpFalse; }

// 值是一个抽象类，表示存储在 SQL 数据中的视图
// 一些物化状态。 所有值都有类型和比较函数，但是
// 子类实现其他特定于类型的功能。
class Value {
  // Friend Type classes
  friend class Type;
  friend class NumericType;
  friend class IntegerParentType;
  friend class TinyintType;
  friend class SmallintType;
  friend class IntegerType;
  friend class BigintType;
  friend class DecimalType;
  friend class TimestampType;
  friend class BooleanType;
  friend class VarlenType;

 public:
  explicit Value(const TypeId type) : manage_data_(false), type_id_(type) { size_.len_ = BUSTUB_VALUE_NULL; }
  // 布尔类型和字节型整数类型（TINYINT）
  Value(TypeId type, int8_t i);
  // 十进制浮点数类型（DECIMAL）
  Value(TypeId type, double d);
  Value(TypeId type, float f);
  // 短整型整数类型（SMALLINT）
  Value(TypeId type, int16_t i);
  // 整型整数类型（INTEGER）
  Value(TypeId type, int32_t i);
  // 长整型整数类型（BIGINT）
  Value(TypeId type, int64_t i);
  // 时间戳类型（TIMESTAMP）
  Value(TypeId type, uint64_t i);
  // 可变长度字符串类型（VARCHAR）

  Value(TypeId type, const char *data, uint32_t len, bool manage_data);
  Value(TypeId type, const std::string &data);

  Value() : Value(TypeId::INVALID) {}
  Value(const Value &other);
  auto operator=(Value other) -> Value &;
  ~Value();
  // NOLINTNEXTLINE
  friend void Swap(Value &first, Value &second) {
    std::swap(first.value_, second.value_);
    std::swap(first.size_, second.size_);
    std::swap(first.manage_data_, second.manage_data_);
    std::swap(first.type_id_, second.type_id_);
  }
  // check whether value is integer
  auto CheckInteger() const -> bool;
  auto CheckComparable(const Value &o) const -> bool;

  // Get the type of this value
  inline auto GetTypeId() const -> TypeId { return type_id_; }

  // Get the length of the variable length data
  inline auto GetLength() const -> uint32_t { return Type::GetInstance(type_id_)->GetLength(*this); }
  // Access the raw variable length data
  inline auto GetData() const -> const char * { return Type::GetInstance(type_id_)->GetData(*this); }

  template <class T>
  inline auto GetAs() const -> T {
    return *reinterpret_cast<const T *>(&value_);
  }

  inline auto CastAs(const TypeId type_id) const -> Value {
    return Type::GetInstance(type_id_)->CastAs(*this, type_id);
  }
  // You will likely need this in project 4...
  inline auto CompareExactlyEquals(const Value &o) const -> bool {
    if (this->IsNull() && o.IsNull()) {
      return true;
    }
    return (Type::GetInstance(type_id_)->CompareEquals(*this, o)) == CmpBool::CmpTrue;
  }
  // Comparison Methods
  inline auto CompareEquals(const Value &o) const -> CmpBool {
    return Type::GetInstance(type_id_)->CompareEquals(*this, o);
  }
  inline auto CompareNotEquals(const Value &o) const -> CmpBool {
    return Type::GetInstance(type_id_)->CompareNotEquals(*this, o);
  }
  inline auto CompareLessThan(const Value &o) const -> CmpBool {
    return Type::GetInstance(type_id_)->CompareLessThan(*this, o);
  }
  inline auto CompareLessThanEquals(const Value &o) const -> CmpBool {
    return Type::GetInstance(type_id_)->CompareLessThanEquals(*this, o);
  }
  inline auto CompareGreaterThan(const Value &o) const -> CmpBool {
    return Type::GetInstance(type_id_)->CompareGreaterThan(*this, o);
  }
  inline auto CompareGreaterThanEquals(const Value &o) const -> CmpBool {
    return Type::GetInstance(type_id_)->CompareGreaterThanEquals(*this, o);
  }

  // Other mathematical functions
  inline auto Add(const Value &o) const -> Value { return Type::GetInstance(type_id_)->Add(*this, o); }
  inline auto Subtract(const Value &o) const -> Value { return Type::GetInstance(type_id_)->Subtract(*this, o); }
  inline auto Multiply(const Value &o) const -> Value { return Type::GetInstance(type_id_)->Multiply(*this, o); }
  inline auto Divide(const Value &o) const -> Value { return Type::GetInstance(type_id_)->Divide(*this, o); }
  inline auto Modulo(const Value &o) const -> Value { return Type::GetInstance(type_id_)->Modulo(*this, o); }
  inline auto Min(const Value &o) const -> Value { return Type::GetInstance(type_id_)->Min(*this, o); }
  inline auto Max(const Value &o) const -> Value { return Type::GetInstance(type_id_)->Max(*this, o); }
  inline auto Sqrt() const -> Value { return Type::GetInstance(type_id_)->Sqrt(*this); }

  inline auto OperateNull(const Value &o) const -> Value { return Type::GetInstance(type_id_)->OperateNull(*this, o); }
  inline auto IsZero() const -> bool { return Type::GetInstance(type_id_)->IsZero(*this); }
  inline auto IsNull() const -> bool { return size_.len_ == BUSTUB_VALUE_NULL; }

  // 将此值序列化到给定的存储空间中。 内联参数
  // 指示是否允许我们将该值内联到存储中
  // 空间，或者我们是否必须仅存储对此值的引用。 如果内联
  // 为 false，我们可以使用提供的数据池为此分配空间
  // value，将引用存储到存储中分配的池空间中。
  inline void SerializeTo(char *storage) const { Type::GetInstance(type_id_)->SerializeTo(*this, storage); }

  // Deserialize a value of the given type from the given storage space.
  inline static auto DeserializeFrom(const char *storage, const TypeId type_id) -> Value {
    return Type::GetInstance(type_id)->DeserializeFrom(storage);
  }

  // Return a string version of this value
  inline auto ToString() const -> std::string { return Type::GetInstance(type_id_)->ToString(*this); }
  // Create a copy of this value
  inline auto Copy() const -> Value { return Type::GetInstance(type_id_)->Copy(*this); }

 protected:
  // The actual value item
  union Val {
    int8_t boolean_;
    int8_t tinyint_;
    int16_t smallint_;
    int32_t integer_;
    int64_t bigint_;
    double decimal_;
    uint64_t timestamp_;
    char *varlen_;
    const char *const_varlen_;
  } value_;

  union {
    uint32_t len_;
    TypeId elem_type_id_;
  } size_;

  bool manage_data_;
  // The data type
  TypeId type_id_;
};
}  // namespace bustub

template <typename T>
struct fmt::formatter<T, std::enable_if_t<std::is_base_of<bustub::Value, T>::value, char>>
    : fmt::formatter<std::string> {
  template <typename FormatCtx>
  auto format(const bustub::Value &x, FormatCtx &ctx) const {
    return fmt::formatter<std::string>::format(x.ToString(), ctx);
  }
};

template <typename T>
struct fmt::formatter<std::unique_ptr<T>, std::enable_if_t<std::is_base_of<bustub::Value, T>::value, char>>
    : fmt::formatter<std::string> {
  template <typename FormatCtx>
  auto format(const std::unique_ptr<bustub::Value> &x, FormatCtx &ctx) const {
    return fmt::formatter<std::string>::format(x->ToString(), ctx);
  }
};
