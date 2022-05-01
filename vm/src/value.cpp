#include <ovis/vm/value.hpp>

namespace ovis {

Value::Value(NotNull<Type*> type) : virtual_machine_(type->virtual_machine()), type_id_(type->id()), is_reference_(false) {
  storage_.Construct(type->virtual_machine()->main_execution_context(), type->description().memory_layout);
}

Value::Value(const Value& other) : virtual_machine_(other.virtual_machine()), type_id_(Type::NONE_ID), is_reference_(false) {
  const auto type = other.type();
  if (!type) {
    return;
  }
  const TypeMemoryLayout& memory_layout = other.is_reference() ? type->description().reference->memory_layout : type->description().memory_layout;
  if (!storage_.Construct(virtual_machine()->main_execution_context(), memory_layout)) {
    return;
  }
  if (!ValueStorage::Copy(virtual_machine()->main_execution_context(), memory_layout, &storage_, &other.storage_)) {
    storage_.Reset(virtual_machine()->main_execution_context());
    return;
  }
  type_id_ = other.type_id_;
  is_reference_ = other.is_reference_;
}

Value& Value::operator=(const Value& other) {
  const TypeMemoryLayout& memory_layout = other.is_reference() ? other.type()->description().reference->memory_layout
                                                               : other.type()->description().memory_layout;
  if (type_id() != other.type_id() || other.virtual_machine() != virtual_machine()) {
    storage_.Reset(virtual_machine()->main_execution_context());
    virtual_machine_ = other.virtual_machine_;
    if (other.type_id_ != Type::NONE_ID) {
      storage_.Construct(virtual_machine()->main_execution_context(), memory_layout);
    }
  }
  assert(virtual_machine() == other.virtual_machine());
  assert(storage_.native_type_id_ == other.storage_.native_type_id_);
  assert(storage_.destruct_function() == other.storage_.destruct_function());
  ValueStorage::Copy(virtual_machine()->main_execution_context(), memory_layout, &storage_, &other.storage_);
  is_reference_ = other.is_reference_;
  type_id_ = other.type_id_;
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

  const auto& reference_description = *type()->description().reference;
  Value value(virtual_machine());
  const auto construct_result = value.storage_.Construct(virtual_machine()->main_execution_context(), reference_description.memory_layout);
  assert(construct_result);

  auto reference_pointer = value.storage_.value_pointer();

  const auto set_pointer_result = reference_description.set_pointer->Call<void>(reference_pointer, value_pointer);
  assert(set_pointer_result);
  value.type_id_ = type_id_;
  value.is_reference_ = true;

  return value;
}

Value::~Value() {
  storage_.Reset(virtual_machine()->main_execution_context());
}

}  // namespace ovis
