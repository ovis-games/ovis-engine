#pragma once

#include <cstring>
#include <memory>
#include <string_view>
#include <type_traits>

#include <ovis/core/type.hpp>
#include <ovis/core/value_storage.hpp>

namespace ovis {

class Value {
 public:
  Value() : type_() {}

  Value(const Value& other);
  Value(Value&&);
  Value& operator=(const Value&);
  Value& operator=(Value&&);

  template <typename T> T& as() { return storage_.as<T>(); }
  const std::shared_ptr<Type>& type() const { return type_; }

  void Reset();

  template <typename T> void SetProperty(std::string_view name, T&& value);
  template <typename T> T GetProperty(std::string_view name);

  template <typename T> static Value Create(T&& native_value);
  static Result<Value> Construct(std::shared_ptr<Type> type);

 private:
  ValueStorage storage_;
  std::shared_ptr<Type> type_;
};

// Implementation
// TODO:
//   * Refactor duplicated code!
//   * Reuse allocated storage

inline Value::Value(const Value& other) : type_(other.type_) {
  if (!type_) {
    return;
  }

  const void* source;
  void* destination;

  if (other.storage_.has_allocated_storage()) {
    storage_.Allocate(type_->alignment_in_bytes(), type_->size_in_bytes());
    source = other.storage_.allocated_storage_pointer();
    destination = storage_.allocated_storage_pointer();
  } else {
    source = other.storage_.data();
    destination = storage_.data();
  }
  if (type_->copy_construct_function()) {
    type_->copy_construct_function()->Call(destination, source);
  } else {
    std::memcpy(destination, source, type_->size_in_bytes());
  }
  storage_.SetDestructFunction(other.storage_.destruct_function());
#ifndef NDEBUG
  storage_.native_type_id_ = other.storage_.native_type_id_;
#endif
}

inline Value::Value(Value&& other) : type_(std::move(other.type_)) {
  if (!type_) {
    return;
  }

  void* source;
  void* destination;

  if (other.storage_.has_allocated_storage()) {
    storage_.Allocate(type_->alignment_in_bytes(), type_->size_in_bytes());
    source = other.storage_.allocated_storage_pointer();
    destination = storage_.allocated_storage_pointer();
  } else {
    source = other.storage_.data();
    destination = storage_.data();
  }
  if (type_->trivially_move_constructible() || type_->trivially_copy_constructible()) {
    std::memcpy(destination, source, type_->size_in_bytes());
  } else {
    type_->move_construct_function()->Call(destination, source);
  }
  storage_.SetDestructFunction(other.storage_.destruct_function());
#ifndef NDEBUG
  storage_.native_type_id_ = other.storage_.native_type_id_;
#endif
}

inline Value& Value::operator=(const Value& other) {
  if (type() == other.type()) {
    const void* source;
    void* destination;
    if (storage_.has_allocated_storage()) {
      assert(other.storage_.has_allocated_storage());
      source = other.storage_.allocated_storage_pointer();
      destination = storage_.allocated_storage_pointer();
    } else {
      source = other.storage_.data();
      destination = storage_.data();
    }
    if (type()->trivially_copy_assignable()) {
      std::memcpy(destination, source, type_->size_in_bytes());
    } else {
      type()->copy_assign_function()->Call(destination, source);
    }
    return *this;
  } else {
    storage_.reset();
    type_ = other.type_;
    if (!type_) {
      return *this;
    }

    const void* source;
    void* destination;

    if (other.storage_.has_allocated_storage()) {
      storage_.Allocate(type_->alignment_in_bytes(), type_->size_in_bytes());
      source = other.storage_.allocated_storage_pointer();
      destination = storage_.allocated_storage_pointer();
    } else {
      source = other.storage_.data();
      destination = storage_.data();
    }
    if (type_->copy_construct_function()) {
      type_->copy_construct_function()->Call(destination, source);
    } else {
      std::memcpy(destination, source, type_->size_in_bytes());
    }
    storage_.SetDestructFunction(other.storage_.destruct_function());
#ifndef NDEBUG
    storage_.native_type_id_ = other.storage_.native_type_id_;
#endif
  }

  return *this;
}

inline Value& Value::operator=(Value&& other) {
  if (type() == other.type()) {
    void* source;
    void* destination;
    if (storage_.has_allocated_storage()) {
      assert(other.storage_.has_allocated_storage());
      source = other.storage_.allocated_storage_pointer();
      destination = storage_.allocated_storage_pointer();
    } else {
      source = other.storage_.data();
      destination = storage_.data();
    }
    if (type()->trivially_move_assignable()) {
      std::memcpy(destination, source, type_->size_in_bytes());
    } else {
      type()->move_assign_function()->Call(destination, source);
    }
    return *this;
  } else {
    storage_.reset();
    type_ = other.type_;
    if (!type_) {
      return *this;
    }

    const void* source;
    void* destination;

    if (other.storage_.has_allocated_storage()) {
      storage_.Allocate(type_->alignment_in_bytes(), type_->size_in_bytes());
      source = other.storage_.allocated_storage_pointer();
      destination = storage_.allocated_storage_pointer();
    } else {
      source = other.storage_.data();
      destination = storage_.data();
    }
    if (type_->copy_construct_function()) {
      type_->copy_construct_function()->Call(destination, source);
    } else {
      std::memcpy(destination, source, type_->size_in_bytes());
    }
    storage_.SetDestructFunction(other.storage_.destruct_function());
#ifndef NDEBUG
    storage_.native_type_id_ = other.storage_.native_type_id_;
#endif
  }

  return *this;
}

inline void Value::Reset() {
  type_.reset();
  storage_.reset();
}

template <typename T>
void Value::SetProperty(std::string_view name, T&& value) {
  const auto property = type()->GetProperty(name);
  assert(property != nullptr);

  const auto& property_type = property->type;
  assert(property_type != nullptr);
  assert(property_type->id() == Type::GetId<T>());

  if (property->access.index() == 0) {
    const auto primitive_access = std::get<TypePropertyDescription::PrimitiveAccess>(property->access);
    auto property_pointer = static_cast<std::byte*>(storage_.value_pointer()) + primitive_access.offset;
    if (property_type->trivially_copy_assignable()) {
      std::memcpy(property_pointer, &value, sizeof(T));
    } else {
      ExecutionContext::global_context()->Call<void>(property_type->copy_assign_function()->handle(), property_pointer, &value);
    }
  } else {
    const auto function_access = std::get<TypePropertyDescription::FunctionAccess>(property->access);
  }
}

template <typename T> T GetProperty(std::string_view name);

template <typename T>
inline Value Value::Create(T&& native_value) {
  Value value;
  value.type_ = Type::Get<std::remove_cvref_t<T>>();
  value.storage_.reset(std::forward<T>(native_value));
  return value;
}

}  // namespace ovis
