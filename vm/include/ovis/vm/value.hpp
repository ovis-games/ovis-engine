#pragma once

#include <cstring>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>

#include "ovis/utils/not_null.hpp"
#include "ovis/vm/type.hpp"
#include "ovis/vm/value_storage.hpp"
#include "ovis/vm/virtual_machine.hpp"

namespace ovis {

class Value {
 public:
  Value(NotNull<VirtualMachine*> virtual_machine)
      : virtual_machine_(virtual_machine), type_id_(Type::NONE_ID), is_reference_(false) {}
  Value(NotNull<VirtualMachine*> virtual_machine, TypeId type_id) : Value(virtual_machine->GetType(type_id)) {}
  Value(NotNull<Type*> type);
  ~Value();

  Value(const Value& other);
  Value& operator=(const Value&);

  NotNull<VirtualMachine*> virtual_machine() const { return virtual_machine_; }

  template <typename T> T& as() {
    assert(GetValuePointer());
    return *reinterpret_cast<T*>(GetValuePointer());
  }
  template <typename T> const T& as() const {
    assert(GetValuePointer());
    return *reinterpret_cast<const T*>(GetValuePointer());
  }

  const void* GetValuePointer() const;
  void* GetValuePointer();

  TypeId type_id() const { return type_id_; }
  Type* type() const { return virtual_machine()->GetType(type_id()); }
  const TypeMemoryLayout* memory_layout() const { return has_value() ? &type()->memory_layout() : nullptr; }
  bool is_reference() const { return is_reference_; }
  bool has_value() const { return type_id() != Type::NONE_ID; }

  Value CreateReference() const;

  void Reset();

  Result<> CopyTo(NotNull<Value*> other) const;
  Result<> CopyTo(NotNull<ExecutionContext*> execution_context, NotNull<Value*> other) const;
  Result<> CopyTo(NotNull<ValueStorage*> storage) const;
  Result<> CopyTo(NotNull<ExecutionContext*> execution_context, NotNull<ValueStorage*> storage) const;

  template <typename T> Result<> SetProperty(std::string_view name, T&& value);
  template <typename T> Result<> SetProperty(ExecutionContext* execution_context, std::string_view name, T&& value);
  template <typename T> Result<T> GetProperty(std::string_view name);
  template <typename T> Result<T> GetProperty(ExecutionContext* execution_context, std::string_view name);

  template <typename T>
  requires (!std::is_pointer_v<T> || std::is_function_v<std::remove_cvref_t<std::remove_pointer_t<T>>>)
  static Value Create(VirtualMachine* virtual_machine, T&& native_value);
  template <typename T>
  requires (std::is_pointer_v<T> && !std::is_function_v<std::remove_cvref_t<std::remove_pointer_t<T>>>)
  static Value Create(VirtualMachine* virtual_machine, T&& native_value);

 private:
  ValueStorage storage_;
  NotNull<VirtualMachine*> virtual_machine_;
  TypeId type_id_;
  bool is_reference_ : 1;
};

}  // namespace ovis

// Implementation
// TODO:
//   * Refactor duplicated code!
//   * Reuse allocated storage
#include <ovis/vm/function.hpp>

namespace ovis {

template <typename T>
Result<> Value::SetProperty(std::string_view name, T&& value) {
  return SetProperty<T>(virtual_machine()->main_execution_context(), name, std::forward<T>(value));
}

template <typename T>
Result<> Value::SetProperty(ExecutionContext* execution_context, std::string_view name, T&& value) {
  using Type = std::remove_cvref_t<T>;

  const auto property = type()->GetProperty(name);
  if (!property) {
    return Error("Type {} does not have a property with the name {}.", type()->name(), name);
  }

  const auto property_type_id = property->type;
  if (property_type_id != virtual_machine()->GetTypeId<Type>()) {
    return Error("Invalid type for property {} of {}, expected `{}` got `{}`", name, type()->name(),
                 virtual_machine()->GetType(property_type_id)->name(), virtual_machine()->GetType<Type>()->name());
  }

  if (property->access.index() == 0) {
    const auto primitive_access = std::get<TypePropertyDescription::PrimitiveAccess>(property->access);
    auto property_pointer = reinterpret_cast<Type*>(static_cast<std::byte*>(storage_.value_pointer()) + primitive_access.offset);
    const auto& property_type = virtual_machine()->GetType(property_type_id);
    if (property_type->trivially_copyable()) {
      std::memcpy(property_pointer, &value, sizeof(Type));
    } else {
      OVIS_CHECK_RESULT(property_type->memory_layout().Copy(property_pointer, &value));
    }
  } else {
    const auto function_access = std::get<TypePropertyDescription::FunctionAccess>(property->access);
    OVIS_CHECK_RESULT(execution_context->Call(function_access.setter->handle(), storage_.value_pointer(), std::forward<T>(value)));
  }

  return Success;
}

template <typename T>
Result<T> Value::GetProperty(std::string_view name) {
  return GetProperty<T>(virtual_machine()->main_execution_context(), name);
}

template <typename T>
Result<T> Value::GetProperty(ExecutionContext* execution_context, std::string_view name) {
  using Type = std::remove_cvref_t<T>;

  const auto property = type()->GetProperty(name);
  if (!property) {
    return Error("Type {} does not have a property with the name {}.", type()->name(), name);
  }

  const auto property_type_id = property->type;
  if (property_type_id != virtual_machine()->GetTypeId<Type>()) {
    return Error("Invalid type for property {} of {}, expected `{}` got `{}`", name, type()->name(),
                 virtual_machine()->GetType(property_type_id)->name(), virtual_machine()->GetType<Type>()->name());
  }

  if (property->access.index() == 0) {
    const auto primitive_access = std::get<TypePropertyDescription::PrimitiveAccess>(property->access);
    auto property_pointer = reinterpret_cast<const Type*>(static_cast<std::byte*>(storage_.value_pointer()) + primitive_access.offset);
    const auto& property_type = virtual_machine()->GetType(property_type_id);

    Type result;
    OVIS_CHECK_RESULT(property_type->memory_layout().Copy(&result, property_pointer));
    return result;
  } else {
    const auto function_access = std::get<TypePropertyDescription::FunctionAccess>(property->access);
    return execution_context->Call<Type>(function_access.setter->handle(), storage_.value_pointer());
  }
}

template <typename T>
requires (!std::is_pointer_v<T> || std::is_function_v<std::remove_cvref_t<std::remove_pointer_t<T>>>)
inline Value Value::Create(VirtualMachine* virtual_machine, T&& native_value) {
  assert(virtual_machine->GetType<std::remove_cvref_t<T>>());

  Value value(virtual_machine);
  value.type_id_ = virtual_machine->GetTypeId<std::remove_cvref_t<T>>();
  value.storage_.Store(std::forward<T>(native_value));
  return value;
}

template <typename T>
requires (std::is_pointer_v<T> && !std::is_function_v<std::remove_cvref_t<std::remove_pointer_t<T>>>)
inline Value Value::Create(VirtualMachine* virtual_machine, T&& native_value) {
  assert(virtual_machine->GetType<std::remove_cvref_t<std::remove_pointer_t<T>>>());

  Value value(virtual_machine);
  value.type_id_ = virtual_machine->GetTypeId<std::remove_cvref_t<std::remove_pointer_t<T>>>();
  value.storage_.Store(std::forward<T>(native_value));
  value.is_reference_ = false;

  return value;
}

}  // namespace ovis
