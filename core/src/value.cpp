#include <ovis/core/value.hpp>

namespace ovis {

Value::Value(const Value& other) : type_(other.type_), is_reference_(other.is_reference_) {
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
  assert(type_->construct_function());
  type_->construct_function()->Call(destination);
  if (type_->trivially_copy_assignable()) {
    std::memcpy(destination, source, type_->size_in_bytes());
  } else {
    type_->copy_assign_function()->Call(destination, source);
  }
  storage_.SetDestructFunction(other.storage_.destruct_function());
#ifndef NDEBUG
  storage_.native_type_id_ = other.storage_.native_type_id_;
#endif
}

Value& Value::operator=(const Value& other) {
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
    is_reference_ = other.is_reference_;
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
    assert(type_->construct_function());
    type_->construct_function()->Call(destination);
    if (type_->trivially_copy_assignable()) {
      std::memcpy(destination, source, type_->size_in_bytes());
    } else {
      type_->copy_assign_function()->Call(destination, source);
    }
    storage_.SetDestructFunction(other.storage_.destruct_function());
#ifndef NDEBUG
    storage_.native_type_id_ = other.storage_.native_type_id_;
#endif
  }

  return *this;
}

Result<Value> Value::Construct(std::shared_ptr<Type> type) {
  assert(type);

  Value value;
  void* stored_value_pointer = value.storage_.AllocateIfNecessary(type->alignment_in_bytes(), type->size_in_bytes());

  assert(type->construct_function());
  const auto constructor_result =
      ExecutionContext::global_context()->Call<void>(type->construct_function()->handle(), stored_value_pointer);
  OVIS_CHECK_RESULT(constructor_result);  // If the constructor failed the storage is freed by the destructor of value

  if (auto destructor = type->destruct_function(); destructor) {
    value.storage_.SetDestructFunction(destructor->handle());
  }

  value.type_ = std::move(type);

  return value;
}

Value::~Value() {}

}
