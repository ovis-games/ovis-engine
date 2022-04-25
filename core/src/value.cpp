#include <ovis/core/value.hpp>

namespace ovis {

Value::Value(const Value& other) : type_(other.type_) {
  if (!type_) {
    return;
  }
  assert(other.type()->description().memory_layout.is_copyable);

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
  if (type_->trivially_copyable()) {
    std::memcpy(destination, source, type_->size_in_bytes());
  } else {
    type_->copy_function()->Call(destination, source);
  }
  storage_.SetDestructFunction(other.storage_.destruct_function());
#ifndef NDEBUG
  storage_.native_type_id_ = other.storage_.native_type_id_;
#endif
}

Value& Value::operator=(const Value& other) {
  if (type() == other.type()) {
    if (!other.type()) {
      return *this;
    }
    assert(other.type()->description().memory_layout.is_copyable);
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
    if (type()->trivially_copyable()) {
      std::memcpy(destination, source, type_->size_in_bytes());
    } else {
      type()->copy_function()->Call(destination, source);
    }
    return *this;
  } else {
    storage_.reset();
    type_ = other.type_;
    if (!type_) {
      return *this;
    }
    assert(other.type()->description().memory_layout.is_copyable);

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
    if (type_->trivially_copyable()) {
      std::memcpy(destination, source, type_->size_in_bytes());
    } else {
      type_->copy_function()->Call(destination, source);
    }
    storage_.SetDestructFunction(other.storage_.destruct_function());
#ifndef NDEBUG
    storage_.native_type_id_ = other.storage_.native_type_id_;
#endif
  }

  return *this;
}

void* Value::GetValuePointer() {
  return const_cast<void*>(static_cast<const Value*>(this)->GetValuePointer());
}

const void* Value::GetValuePointer() const {
  if (!is_reference()) {
    return storage_.value_pointer();
  }

  auto value_pointer = type()->description().reference->get_pointer->Call<void*>(storage_.value_pointer());
  assert(value_pointer);
  return *value_pointer;
}

Value Value::CreateReference() const {
  assert(type());
  assert(type()->is_reference_type());

  const void* value_pointer = GetValuePointer();

  const auto& reference_description = type()->description().reference;
  Value value;
  void* reference_pointer = value.storage_.AllocateIfNecessary(reference_description->memory_layout.alignment_in_bytes,
                                                               reference_description->memory_layout.size_in_bytes);
  const auto constructor_result = reference_description->memory_layout.construct->Call<void>(reference_pointer);
  assert(constructor_result);

  const auto set_pointer_result = reference_description->set_pointer->Call<void>(reference_pointer, value_pointer);
  assert(set_pointer_result);
  value.type_ = type();
  value.is_reference_ = true;

  return value;
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
