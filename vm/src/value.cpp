#include "ovis/vm/value.hpp"

#include "ovis/vm/type.hpp"
#include "ovis/vm/virtual_machine.hpp"

namespace ovis {

Value::Value(NotNull<VirtualMachine*> virtual_machine)
    : virtual_machine_(virtual_machine), type_id_(Type::NONE_ID), is_reference_(false) {}

Value::Value(NotNull<VirtualMachine*> virtual_machine, TypeId type_id) : Value(virtual_machine->GetType(type_id)) {}

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

  if (other->type_id() != type_id() || !other->is_reference() != !is_reference()) {
    other->Reset();
    if (!is_reference()) {
      OVIS_CHECK_RESULT(other->storage_.Construct(execution_context, *memory_layout()));
    }
  }
  other->type_id_ = type_id();
  other->is_reference_ = is_reference();

  if (is_reference()) {
    ValueStorage::CopyTrivially(&other->storage_, &storage_);
  } else {
    OVIS_CHECK_RESULT(ValueStorage::Copy(execution_context, *memory_layout(), &other->storage_, &storage_));
  }
  return Success;
}

Result<> Value::CopyTo(NotNull<ValueStorage*> storage) const {
  return CopyTo(virtual_machine()->main_execution_context(), storage);
}

Result<> Value::CopyTo(NotNull<ExecutionContext*> execution_context, NotNull<ValueStorage*> storage) const {
  assert(has_value());
  assert(!is_reference() && "Not implemented yet");
  OVIS_CHECK_RESULT(storage->Construct(execution_context, type()->memory_layout()));
  OVIS_CHECK_RESULT(ValueStorage::Copy(execution_context, type()->memory_layout(), storage, &storage_));
  return Success;
}

void* Value::GetValuePointer() {
  return const_cast<void*>(static_cast<const Value*>(this)->GetValuePointer());
}

const void* Value::GetValuePointer() const {
  if (!is_reference()) {
    return storage_.value_pointer();
  } else {
    return storage_.as<void*>();
  }
}

Type* Value::type() const {
  return virtual_machine()->GetType(type_id());
}

const TypeMemoryLayout* Value::memory_layout() const {
  return has_value() ? &type()->memory_layout() : nullptr;
}

bool Value::has_value() const { return type_id() != Type::NONE_ID; }

Value Value::CreateReference() const {
  assert(!is_reference());
  static_assert(ValueStorage::stored_inline<decltype(GetValuePointer())>);

  Value reference_value(virtual_machine());
  reference_value.storage_.Store(GetValuePointer());
  reference_value.is_reference_ = true;
  reference_value.type_id_ = type_id();
  assert(!reference_value.storage_.has_allocated_storage());

  return reference_value;
}

Value::~Value() {
  storage_.Reset(virtual_machine()->main_execution_context());
}

Result<> Value::SetProperty(std::string_view name, NativeTypeId type_id, const void* value) {
  return SetProperty(virtual_machine()->main_execution_context(), name, type_id, value);
}

Result<> Value::SetProperty(ExecutionContext* execution_context, std::string_view name, NativeTypeId native_type_id, const void* value) {
  const auto property = type()->GetProperty(name);
  if (!property) {
    return Error("Type {} does not have a property with the name {}.", type()->name(), name);
  }

  const auto property_type_id = property->type;
  if (property_type_id != virtual_machine()->GetTypeId(native_type_id)) {
    return Error("Invalid type for property {} of {}, expected `{}` got `{}`", name, type()->name(),
                 virtual_machine()->GetType(property_type_id)->name(),
                 virtual_machine()->GetType(native_type_id)->name());
  }

  if (property->access.index() == 0) {
    const auto primitive_access = std::get<TypePropertyDescription::PrimitiveAccess>(property->access);
    auto property_pointer = static_cast<std::byte*>(storage_.value_pointer()) + primitive_access.offset;
    const auto& property_type = virtual_machine()->GetType(property_type_id);
    return property_type->memory_layout().Copy(property_pointer, value);
  } else {
    assert(false && "Not implemented");
    // const auto function_access = std::get<TypePropertyDescription::FunctionAccess>(property->access);
    // OVIS_CHECK_RESULT(execution_context->Call(function_access.setter->handle(), storage_.value_pointer(), value));
  }

  return Success;
}

Result<> Value::GetProperty(std::string_view name, NativeTypeId type_id, void* value) {
  return GetProperty(virtual_machine()->main_execution_context(), name, type_id, value);
}

Result<> Value::GetProperty(ExecutionContext* execution_context, std::string_view name, NativeTypeId native_type_id, void* value) {
  const auto property = type()->GetProperty(name);
  if (!property) {
    return Error("Type {} does not have a property with the name {}.", type()->name(), name);
  }

  const auto property_type_id = property->type;
  if (property_type_id != virtual_machine()->GetTypeId(native_type_id)) {
    return Error("Invalid type for property {} of {}, expected `{}` got `{}`", name, type()->name(),
                 virtual_machine()->GetType(property_type_id)->name(),
                 virtual_machine()->GetType(native_type_id)->name());
  }

  if (property->access.index() == 0) {
    const auto primitive_access = std::get<TypePropertyDescription::PrimitiveAccess>(property->access);
    auto property_pointer = static_cast<std::byte*>(storage_.value_pointer()) + primitive_access.offset;
    const auto& property_type = virtual_machine()->GetType(property_type_id);

    return property_type->memory_layout().Copy(value, property_pointer);
  } else {
    assert(false && "Not implemented");
    // const auto function_access = std::get<TypePropertyDescription::FunctionAccess>(property->access);
    // return execution_context->Call<Type>(function_access.setter->handle(), storage_.value_pointer());
  }
}

}  // namespace ovis
