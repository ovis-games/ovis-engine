#include <ovis/core/value.hpp>

namespace ovis {

Value::Value(NotNull<Type*> type) : virtual_machine_(type->virtual_machine()), type_id_(type->id()), is_reference_(false) {
  void* stored_value_pointer = storage_.AllocateIfNecessary(type->alignment_in_bytes(), type->size_in_bytes());

  assert(type->construct_function());
  const auto constructor_result = virtual_machine()->main_execution_context()->Call<void>(
      type->construct_function()->handle(), stored_value_pointer);
  if (!constructor_result) {
    storage_.Deallocate();
  }

  if (auto destructor = type->destruct_function(); destructor) {
    storage_.SetDestructFunction(destructor->handle());
  }
}

Value::Value(const Value& other) : virtual_machine_(other.virtual_machine()), type_id_(other.type_id()), is_reference_(other.is_reference()) {
  const auto type = this->type();
  if (!type) {
    return;
  }
  const auto& memory_layout =
      other.is_reference() ? type->description().reference->memory_layout : type->description().memory_layout;
  assert(memory_layout.is_copyable);

  const void* source;
  void* destination;

  if (other.storage_.has_allocated_storage()) {
    storage_.Allocate(memory_layout.alignment_in_bytes, memory_layout.size_in_bytes);
    source = other.storage_.allocated_storage_pointer();
    destination = storage_.allocated_storage_pointer();
  } else {
    source = other.storage_.data();
    destination = storage_.data();
  }
  assert(memory_layout.construct);
  memory_layout.construct->Call(destination);
  if (memory_layout.copy) {
    memory_layout.copy->Call(destination, source);
  } else {
    std::memcpy(destination, source, memory_layout.size_in_bytes);
  }
  storage_.SetDestructFunction(other.storage_.destruct_function());
#ifndef NDEBUG
  storage_.native_type_id_ = other.storage_.native_type_id_;
#endif
}

Value& Value::operator=(const Value& other) {
  if (type_id() == other.type_id()) {
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
    
    auto type = this->type();
    if (type->trivially_copyable()) {
      std::memcpy(destination, source, type->size_in_bytes());
    } else {
      type->copy_function()->Call(destination, source);
    }
    return *this;
  } else {
    storage_.Reset(virtual_machine()->main_execution_context());
    type_id_ = other.type_id_;

    auto type = this->type();
    if (!type) {
      return *this;
    }
    assert(other.type()->description().memory_layout.is_copyable);

    const void* source;
    void* destination;

    if (other.storage_.has_allocated_storage()) {
      storage_.Allocate(type->alignment_in_bytes(), type->size_in_bytes());
      source = other.storage_.allocated_storage_pointer();
      destination = storage_.allocated_storage_pointer();
    } else {
      source = other.storage_.data();
      destination = storage_.data();
    }
    assert(type->construct_function());
    type->construct_function()->Call(destination);
    if (type->trivially_copyable()) {
      std::memcpy(destination, source, type->size_in_bytes());
    } else {
      type->copy_function()->Call(destination, source);
    }
    storage_.SetDestructFunction(other.storage_.destruct_function());
#ifndef NDEBUG
    storage_.native_type_id_ = other.storage_.native_type_id_;
#endif
  }

  return *this;
}

void Value::Reset() {
  type_id_ = Type::NONE_ID;
  storage_.Reset(virtual_machine()->main_execution_context());
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
  Value value(virtual_machine());
  void* reference_pointer = value.storage_.AllocateIfNecessary(reference_description->memory_layout.alignment_in_bytes,
                                                               reference_description->memory_layout.size_in_bytes);
  const auto constructor_result = reference_description->memory_layout.construct->Call<void>(reference_pointer);
  assert(constructor_result);

  const auto set_pointer_result = reference_description->set_pointer->Call<void>(reference_pointer, value_pointer);
  assert(set_pointer_result);
  value.type_id_ = type_id_;
  value.is_reference_ = true;

  return value;
}

Value::~Value() {}

}
