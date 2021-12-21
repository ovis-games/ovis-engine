#pragma once

#include <memory>
#include <type_traits>

#include <ovis/core/type.hpp>
#include <ovis/core/value_storage.hpp>

namespace ovis {

class Value {
 public:
  Value() : type_() {}
  template <typename T> requires(!std::is_same_v<Value, std::remove_cvref_t<T>>) Value(T&& value);

  Value(const Value& other);
  Value(Value&&);
  Value& operator=(const Value&);
  Value& operator=(Value&&);

  template <typename T> T& as() { return storage_.as<T>(); }
  const std::shared_ptr<Type>& type() const { return type_; }

 private:
  ValueStorage storage_;
  std::shared_ptr<Type> type_;
};

// Implementation
// TODO:
//   * Refactor duplicated code!
//   * Reuse allocated storage
template <typename T> requires(!std::is_same_v<Value, std::remove_cvref_t<T>>)
Value::Value(T&& value) : type_(Type::Get<std::remove_cvref_t<T>>()) {
  storage_.reset(std::forward<T>(value));
}

inline Value::Value(const Value& other) : type_(other.type_) {
  if (!type_) {
    return;
  }
  assert(type_->copy_constructible());
  if (!type_->copy_constructible()) {
    type_ = nullptr;
    return;
  }

  const void* source;
  void* destination;

  if (other.storage_.allocated_storage()) {
    storage_.Allocate(type_->alignment_in_bytes(), type_->size_in_bytes());
    source = other.storage_.data_as_pointer();
    destination = storage_.data_as_pointer();
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
  assert(type_->move_constructible() || type_->copy_constructible());
  if (!type_->move_constructible() && !type_->copy_constructible()) {
    type_ = nullptr;
    return;
  }

  void* source;
  void* destination;

  if (other.storage_.allocated_storage()) {
    storage_.Allocate(type_->alignment_in_bytes(), type_->size_in_bytes());
    source = other.storage_.data_as_pointer();
    destination = storage_.data_as_pointer();
  } else {
    source = other.storage_.data();
    destination = storage_.data();
  }
  if (type_->trivially_move_constructible() || type_->trivially_copy_constructible()) {
    std::memcpy(destination, source, type_->size_in_bytes());
  } else if (type_->move_constructible()) {
    type_->move_construct_function()->Call(destination, source);
  } else {
    assert(type_->copy_construct_function());
    type_->copy_construct_function()->Call(destination, source);
  }
  storage_.SetDestructFunction(other.storage_.destruct_function());
#ifndef NDEBUG
  storage_.native_type_id_ = other.storage_.native_type_id_;
#endif
}

inline Value& Value::operator=(const Value& other) {
  if (type() == other.type()) {
    assert(type()->copy_assignable());
    const void* source;
    void* destination;
    if (storage_.allocated_storage()) {
      assert(other.storage_.allocated_storage());
      source = other.storage_.data_as_pointer();
      destination = storage_.data_as_pointer();
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

    assert(type_->copy_constructible());
    if (!type_->copy_constructible()) {
      type_ = nullptr;
      return *this;
    }

    const void* source;
    void* destination;

    if (other.storage_.allocated_storage()) {
      storage_.Allocate(type_->alignment_in_bytes(), type_->size_in_bytes());
      source = other.storage_.data_as_pointer();
      destination = storage_.data_as_pointer();
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
    assert(type()->move_assignable());
    void* source;
    void* destination;
    if (storage_.allocated_storage()) {
      assert(other.storage_.allocated_storage());
      source = other.storage_.data_as_pointer();
      destination = storage_.data_as_pointer();
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

    assert(type_->copy_constructible());
    if (!type_->copy_constructible()) {
      type_ = nullptr;
      return *this;
    }

    const void* source;
    void* destination;

    if (other.storage_.allocated_storage()) {
      storage_.Allocate(type_->alignment_in_bytes(), type_->size_in_bytes());
      source = other.storage_.data_as_pointer();
      destination = storage_.data_as_pointer();
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

}  // namespace ovis
