#include <ovis/vm/value.hpp>

namespace ovis {

Value::Value(NotNull<Type*> type) : virtual_machine_(type->virtual_machine()), type_id_(type->id()), is_reference_(false) {
  storage_.Construct(type->virtual_machine()->main_execution_context(), type->description().memory_layout);
}

Value::Value(const Value& other) : virtual_machine_(other.virtual_machine()), type_id_(Type::NONE_ID), is_reference_(false) {
  if (other.has_value()) {
    other.CopyTo(this);
  }
}

Value& Value::operator=(const Value& other) {
  if (other.has_value()) {
    other.CopyTo(this);
  }
  return *this;
}

void Value::Reset() {
  if (has_value()) {
    type_id_ = Type::NONE_ID;
    storage_.Reset(virtual_machine()->main_execution_context());
  }
}

Result<> Value::CopyTo(NotNull<Value*> other) const {
  return CopyTo(virtual_machine()->main_execution_context(), other);
}

Result<> Value::CopyTo(NotNull<ExecutionContext*> execution_context, NotNull<Value*> other) const {
  assert(has_value());
  assert(memory_layout());

  // Technical this would be allowed, however it seems that this might be a mistake? If not remove the assertion and
  // assign the new virtual machine (and please write a test for it!).
  assert(other->virtual_machine() == virtual_machine());

  if (other->type_id() != type_id() || other->is_reference() != is_reference()) {
    other->Reset();
    OVIS_CHECK_RESULT(other->storage_.Construct(execution_context, *memory_layout()));
    other->type_id_ = type_id();
    other->is_reference_ = is_reference();
  }
  OVIS_CHECK_RESULT(ValueStorage::Copy(execution_context, *memory_layout(), &other->storage_, &storage_));
  return Success;
}

Result<> Value::CopyTo(NotNull<ValueStorage*> storage) const {
  return CopyTo(virtual_machine()->main_execution_context(), storage);
}

Result<> Value::CopyTo(NotNull<ExecutionContext*> execution_context, NotNull<ValueStorage*> storage) const {
  assert(has_value());
  const TypeMemoryLayout& memory_layout =
      is_reference() ? type()->description().reference->memory_layout : type()->memory_layout();
  OVIS_CHECK_RESULT(storage->Construct(execution_context, memory_layout));
  OVIS_CHECK_RESULT(ValueStorage::Copy(execution_context, memory_layout, storage, &storage_));
  return Success;
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
